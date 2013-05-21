#include "network.hpp"

#include <sstream>


#include "../win32/network/network_helper.hpp"
#include "../memory_pool/sgi_memory_pool.hpp"
#include "../memory_pool/fixed_memory_pool.hpp"
#include "../extend_stl/allocator/container_allocator.hpp"



namespace async { namespace network {

	std::string error_msg(std::error_code err)
	{
		std::ostringstream oss;
		char *buffer = 0;

		DWORD ret = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			0, err.value(), 0, (LPSTR)&buffer, 0, 0);
		if( ret == 0 )
			return std::string();

		oss << "Win32 Error(" << err << ") : " << buffer;

		::LocalFree(buffer);
		return std::move(oss.str());
	}

	void __stdcall APCFunc(ULONG_PTR)
	{
		// do nothing
	}


	session::session(service::io_dispatcher_t &io, socket_handle_t &&sck, 
		const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler)
		: io_(io)
		, sck_(std::move(sck))
		, data_(nullptr)
		, error_handler_(error_handler)
		, disconnect_handler_(disconnect_handler)
	{
		sck_.set_option(network::linger(true, 0));
		sck_.set_option(network::no_delay(true));
	}
	session::~session()
	{
		sck_.close();
	}

	std::string session::get_ip() const
	{
		return win32::network::ip_2_string(win32::network::get_sck_ip(sck_.native_handle()));
	}

	void session::additional_data(void *data)
	{
		data_ = data;
	}

	void *session::additional_data() const
	{
		return data_; 
	}

	template < typename HandlerT >
	void session::_handle_read(std::error_code error, std::uint32_t size, const HandlerT &read_handler)
	{
		try
		{
			if( error == 0 )	// success
			{
				if( size == 0 )
				{
					// disconnect
					disconnect();
				}
				else if( read_handler )
					read_handler(shared_from_this(), size);
			}
			else
			{
				if( error_handler_ )
				{
					error_handler_(shared_from_this(), std::cref(error_msg(error)));
				}
				disconnect();
			}
		}
		catch(...)
		{
			assert(0 && "has an exception in read_handler_");
			error_handler_(shared_from_this(), "has an exception in read_handler");
		}
	}

	template < typename HandlerT >
	void session::_handle_write(std::error_code error, std::uint32_t size, const HandlerT &write_handler)
	{
		try
		{
			if( error == 0 )	// success
			{
				if( size == 0 )
				{
					disconnect();
				}
				else if( write_handler != 0 )
					write_handler(shared_from_this(), size);
			}
			else
			{
				if( error_handler_ )
					error_handler_(shared_from_this(), std::cref(error_msg(error)));
				disconnect();
			}
		}
		catch(...)
		{
			assert(0 && "has an exception in write_handler_");
			error_handler_(shared_from_this(), "has an exception in write_handler_");
		}
	}

	void session::disconnect()
	{
		try
		{
			if( disconnect_handler_ )
				disconnect_handler_(shared_from_this());
		}
		catch(...)
		{
			assert(0 && "has an exception in disconnect_handler_");
			error_handler_(shared_from_this(), "has an exception in disconnect_handler_");
		}

		sck_.close();
	}


	
	template < typename IOT, typename SocketT >
	session_ptr create_session(IOT &io, 
		SocketT &&sck,
		const error_handler_type &error_handler, 
		const disconnect_handler_type &disconnect_handler)
	{
		typedef memory_pool::sgi_memory_pool_t<false, 2 * sizeof(session)> pool_t;
		static pool_t pool;


		typedef stdex::allocator::pool_allocator_t<session, pool_t> pool_allocator_t;
		pool_allocator_t pool_allocate(pool);
		return std::allocate_shared<session>(pool_allocate, 
			io, 
			std::forward<SocketT>(sck), 
			error_handler, 
			disconnect_handler);
	}


	struct server::impl
	{
		service::io_dispatcher_t io_;
		tcp::accpetor acceptor_;

		error_handler_type error_handle_;
		accept_handler_type accept_handle_;
		disconnect_handler_type disconnect_handle_;

		std::shared_ptr<std::thread> thread_;

		impl(std::uint16_t port)
			: io_(1)
			, acceptor_(io_, tcp::v4(), port, INADDR_ANY, true)
		{
		}

		void _handle_accept(std::error_code error, socket_handle_t &remote_sck, const SOCKADDR_IN *remote_addr)
		{
			session_ptr val = create_session(io_, std::move(remote_sck), error_handle_, disconnect_handle_);

			if( error != 0 )
			{
				error_handle_(std::cref(val), std::cref(error_msg(error)));
				return;
			}

			std::string address = win32::network::ip_2_string(remote_addr->sin_addr.S_un.S_addr);

			if( accept_handle_ != nullptr )
			{
				accept_handle_(std::cref(val), std::cref(address));
			}
		}

		void _thread_impl()
		{
			static const std::uint32_t MAX_ACCEPT_NUM = 100;

			// 通过使用WSAEventSelect来判断是否有足够的AcceptEx，或者检测出一个非正常的客户请求
			HANDLE accept_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			::WSAEventSelect(acceptor_.native_handle(), accept_event, FD_ACCEPT);


			// async accept memory pool
			typedef memory_pool::sgi_memory_pool_t<false, 256> pool_t;
			typedef stdex::allocator::pool_allocator_t<char, pool_t> pool_allocator_t;

			pool_t pool;
			pool_allocator_t pool_allocator(pool);

			while(true)
			{	
				DWORD ret = ::WaitForSingleObjectEx(accept_event, INFINITE, TRUE);
				if( ret == WAIT_FAILED || ret == WAIT_IO_COMPLETION )
					break;
				else if( ret != WAIT_OBJECT_0 )
					continue;

				// 投递接收连接
				tcp v4_ver = tcp::v4();

				for(std::uint32_t i = 0; i != MAX_ACCEPT_NUM; ++i)
				{
					try
					{
						socket_handle_t sck(io_, v4_ver.family(), v4_ver.type(), v4_ver.protocol());

						acceptor_.async_accept(std::move(sck), pool_allocator,
							std::bind(&impl::_handle_accept, 
							this, 
							service::_Error, 
							service::_Socket,
							service::_Address));
					}
					catch(std::exception &e)
					{
						if( error_handle_ )
							error_handle_(session_ptr(), std::cref(std::string(e.what())));
					}
				}

			}

			::CloseHandle(accept_event);
		}
	};


	server::server(std::uint16_t port)
		: impl_(new impl(port))
	{

	}

	server::~server()
	{

	}

	bool server::start()
	{
		impl_->thread_ = std::make_shared<std::thread>(std::bind(&impl::_thread_impl, impl_.get()));
		return true;
	}

	bool server::stop()
	{
		if( impl_->thread_ )
		{
			::QueueUserAPC(APCFunc, impl_->thread_->native_handle(), 0);
			impl_->thread_->join();
		}

		impl_->io_.stop();
		impl_->acceptor_.close();

		impl_->error_handle_	= nullptr;
		impl_->accept_handle_	= nullptr;

		return true;
	}

	void server::register_error_handler(const error_handler_type &handler)
	{
		impl_->error_handle_ = handler;
	}

	void server::register_accept_handler(const accept_handler_type &handler)
	{
		impl_->accept_handle_ = handler;
	}

	void server::register_disconnect_handler(const disconnect_handler_type &handler)
	{
		impl_->disconnect_handle_ = handler;
	}

	// ----------------------------------

	client::client(service::io_dispatcher_t &io, timer::win_timer_service_t &timer_svr)
		: io_(io)
		, socket_(io_, network::tcp::v4())
		, timer_svr_(timer_svr)
	{

	}

	client::~client()
	{
		timer_svr_.stop();
	}

	void client::register_error_handler(const error_handler_type &handler)
	{
		error_handle_ = handler;
	}

	void client::register_connect_handler(const connect_handler_type &handler)
	{
		connect_handle_ = handler;
	}

	void client::register_disconnect_handler(const disconnect_handler_type &handler)
	{
		disconnect_handle_ = handler;
	}


	bool client::start(const std::string &ip, std::uint16_t port, std::chrono::seconds time_out /* = 0 */)
	{
		// 允许为空
			/*assert(error_handle_ != 0);
			assert(connect_handle_ != 0);
			assert(disconnect_handle_ != 0);
			assert(read_handle_ != 0);
			assert(write_handle_ != 0);*/

			try
			{
				if( !socket_.is_open() )
				{
					socket_.open();
				}

				socket_.set_option(network::no_delay(true));
				socket_.set_option(network::linger(true, 0));
				//bool suc = socket_.set_option(network::recv_time_out(time_out * 1000));
				//suc = socket_.set_option(network::send_time_out(time_out * 1000));

				timer_.reset(new timer::timer_handle(timer_svr_));

				/*std::weak_ptr<impl> weak_this = shared_from_this();
				timer_->async_wait([weak_this]()
				{ 
				auto shared_this = weak_this.lock();
				if( shared_this )
				shared_this->_disconnect(); 
				}, time_out, time_out);*/

				//socket_.async_connect(network::ip_address::parse(ip), port,
				//	std::bind(&impl::_on_connect, shared_from_this(), service::_Error));

				socket_.connect(port, network::ip_address::parse(ip));
				_on_connect(std::make_error_code((std::errc::errc)::GetLastError()));
			}
			catch(exception::exception_base &e)
			{
				e.dump();

				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));
				return false;
			}
			catch(std::exception &e)
			{
				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));
				return false;
			}

			return true;
	}

	void client::stop()
	{
		socket_.close();
	}


	std::uint32_t client::send(const char *buf, std::uint32_t len)
	{
		try
		{
			return service::write(socket_, service::buffer(buf, len));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return 0;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return 0;
		}
	}

	std::uint32_t client::recv(char *buf, std::uint32_t len)
	{
		try
		{
			return service::read(socket_, service::buffer(buf, len));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return 0;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return 0;
		}
	}

	void client::disconnect()
	{
		try
		{
			if( disconnect_handle_ )
				disconnect_handle_();
		}
		catch(...)
		{
			assert(0 && "has an exception in disconnect_handler_");
		}
	}

	void client::_on_connect(std::error_code error)
	{
		try
		{
			if( connect_handle_ )
				connect_handle_(error.value() == 0);

			if( error == 0 )
			{
				//timer_->cancel();
			}
			else
			{
				if( error_handle_ )
					error_handle_(std::cref(error_msg(error)));
				disconnect();
			}
		}
		catch(...)
		{
			assert(0 && "has an exception in connect_handler_");
		}
	}

}
}
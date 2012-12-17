#include "network.hpp"

#include <sstream>
#include <iostream>

#include "async_io/iocp/dispatcher.hpp"
#include "async_io/network/tcp.hpp"
#include "async_io/timer/timer.hpp"

#include "multi_thread/thread.hpp"
#include "win32/network/network_helper.hpp"




namespace wx
{
	using namespace async;


	namespace
	{
		std::string error_msg(iocp::error_code err)
		{
			std::ostringstream oss;
			char *buffer = 0;

			DWORD ret = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				0, iocp::error_code_2_win32_error(err), 0, (LPSTR)&buffer, 0, 0);
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
	}

	class session
		: public std::enable_shared_from_this<session>
	{
		iocp::io_dispatcher &io_;
		network::tcp::socket sck_;

		iocp::rw_callback_type read_callback_;
		iocp::rw_callback_type write_callback_;

	public:
		const error_handler_type &error_handler_;
		read_handler_type read_handler_;
		write_handler_type write_handler_;
		disconnect_handler_type disconnect_handler_;

	public:
		session(iocp::io_dispatcher &io, const network::socket_handle_ptr &sck, 
			const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler)
			: io_(io)
			, sck_(sck)
			, error_handler_(error_handler)
			, disconnect_handler_(disconnect_handler)
		{
			sck_.set_option(network::linger(true, 0));
		}
		~session()
		{
			sck_.close();
		}
	

	public:
		const network::tcp::socket &get() const
		{
			return sck_;
		}

		void start()
		{
			read_callback_	= std::bind(&session::_handle_read, shared_from_this(), iocp::_Error, iocp::_Size);
			write_callback_ = std::bind(&session::_handle_write, shared_from_this(), iocp::_Error, iocp::_Size);
		}

		void stop()
		{
			read_handler_	= 0;
			write_handler_	= 0;
			disconnect_handler_ = 0;

			read_callback_	= 0;
			write_callback_ = 0;
		}
		
		bool async_read(iocp::mutable_buffer &buffer)
		{
			try
			{
				iocp::async_read(sck_, buffer, iocp::transfer_all(), read_callback_);
			}
			catch(exception::exception_base &e)
			{
				e.dump();

				if( error_handler_ )
					error_handler_(shared_from_this(), std::cref(std::string(e.what())));

				_disconnect();

				return false;
			}
			catch(std::exception &e)
			{
				if( error_handler_ )
					error_handler_(shared_from_this(), std::cref(std::string(e.what())));

				_disconnect();
				return false;
			}

			return true;
		}

		bool async_write(const iocp::const_buffer &buffer)
		{
			try
			{
				//sck_.async_write(buffer, write_callback_);
				iocp::async_write(sck_, buffer, iocp::transfer_all(), write_callback_);
			}
			catch(exception::exception_base &e)
			{
				e.dump();

				if( error_handler_ )
					error_handler_(shared_from_this(), std::cref(std::string(e.what())));
				_disconnect();

				return false;
			}
			catch(std::exception &e)
			{
				if( error_handler_ )
					error_handler_(shared_from_this(), std::cref(std::string(e.what())));

				_disconnect();
				return false;
			}

			return true;
		}

	private:
		void _handle_read(iocp::error_code error, u_long size)
		{
			try
			{
				if( error == 0 )	// success
				{
					if( size == 0 )
					{
						// disconnect
						_disconnect();
					}
					else if( read_handler_ )
						read_handler_(shared_from_this(), size);
				}
				else
				{
					if( error_handler_ )
						error_handler_(shared_from_this(), std::cref(error_msg(error)));
					_disconnect();
				}
			}
			catch(...)
			{
				assert(0 && "has an exception in read_handler_");
			}
		}

		void _handle_write(iocp::error_code error, u_long size)
		{
			try
			{
				if( error == 0 )	// success
				{
					if( size == 0 )
					{
						_disconnect();
					}
					else if( write_handler_ != 0 )
						write_handler_(shared_from_this(), size);
				}
				else
				{
					if( error_handler_ )
						error_handler_(shared_from_this(), std::cref(error_msg(error)));
					_disconnect();
				}
			}
			catch(...)
			{
				assert(0 && "has an exception in write_handler_");
			}
		}

		void _disconnect()
		{
			try
			{
				if( disconnect_handler_ )
					disconnect_handler_(shared_from_this());
			}
			catch(...)
			{
				assert(0 && "has an exception in disconnect_handler_");
			}

			stop();
		}
	};

}

// 定制自己的工厂
namespace async
{
	namespace iocp
	{
		template<>
		struct object_factory_t< wx::session >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(wx::session)>	PoolType;
			typedef object_pool_t< PoolType >									ObjectPoolType;
		};
	}
}



namespace wx
{
	session_ptr create_session(iocp::io_dispatcher &io, const network::socket_handle_ptr &sck,
		const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler)
	{
		return session_ptr(iocp::object_allocate<session>(io, sck, error_handler, disconnect_handler), 
			&iocp::object_deallocate<session>);
	}


	struct server::impl
	{
		iocp::io_dispatcher io_;
		network::tcp::accpetor acceptor_;

		error_handler_type error_handle_;
		accept_handler_type accept_handle_;
		disconnect_handler_type disconnect_handle_;

		multi_thread::thread_impl_ex thread_;

		impl(std::uint16_t port)
			: acceptor_(io_, network::tcp::v4(), port, INADDR_ANY, true)
		{
		}

		void _handle_accept(iocp::error_code error, const network::socket_handle_ptr &remote_sck, const SOCKADDR_IN *remote_addr)
		{
			session_ptr val(create_session(io_, remote_sck, error_handle_, disconnect_handle_));

			if( error != 0 )
			{
				error_handle_(std::cref(val), std::cref(error_msg(error)));
				return;
			}

			std::string &&address = win32::network::ip_2_string(remote_addr->sin_addr.S_un.S_addr);

			val->start();

			if( accept_handle_ != 0 )
			{
				if( !accept_handle_(std::cref(val), std::cref(address)) )
					val->stop();
			}
		}

		DWORD _thread_impl()
		{
			static const std::uint32_t MAX_ACCEPT_NUM = 10;

			// 通过使用WSAEventSelect来判断是否有足够的AcceptEx，或者检测出一个非正常的客户请求
			HANDLE accept_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			::WSAEventSelect(acceptor_.native_handle(), accept_event, FD_ACCEPT);

			while(!thread_.is_aborted())
			{	
				DWORD ret = ::WaitForSingleObjectEx(accept_event, INFINITE, TRUE);
				if( ret == WAIT_FAILED || ret == WAIT_IO_COMPLETION )
					break;
				else if( ret != WAIT_OBJECT_0 )
					continue;

				// 投递接收连接
				network::tcp v4_ver = network::tcp::v4();

				for(std::uint32_t i = 0; i != MAX_ACCEPT_NUM; ++i)
				{
					try
					{
						network::socket_handle_ptr sck(network::make_socket(io_, 
							v4_ver.family(), v4_ver.type(), v4_ver.protocol()));

						acceptor_.async_accept(sck, std::bind(&impl::_handle_accept, this, iocp::_Error, iocp::_Socket, iocp::_Address));
					}
					catch(std::exception &e)
					{
						if( error_handle_ )
							error_handle_(session_ptr(), std::cref(std::string(e.what())));
					}
				}

			}

			::CloseHandle(accept_event);
			return 0;
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
		impl_->thread_.register_callback(std::bind(&impl::_thread_impl, impl_.get()));
		impl_->thread_.start();
		
		return true;
	}

	bool server::stop()
	{
		::QueueUserAPC(APCFunc, impl_->thread_, 0);
		impl_->thread_.stop();
		impl_->io_.stop();
		impl_->acceptor_.close();

		impl_->error_handle_	= 0;
		impl_->accept_handle_	= 0;

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



	bool async_read(const session_ptr &val, char *buf, std::uint32_t len)
	{
		if( !val )
			return false;

		return val->async_read(iocp::buffer(buf, len));
	}

	bool async_write(const session_ptr &val, const char *buf, std::uint32_t len)
	{
		if( !val )
			return false;

		return val->async_write(iocp::buffer(buf, len));
	}

	void close(const session_ptr &val)
	{
		if( val )
			val->stop();
	}

	std::string get_ip(const session_ptr &session)
	{
		return win32::network::ip_2_string(win32::network::get_sck_ip(session->get().native_handle()));
	}

	void register_read_handler(const session_ptr &remote, const read_handler_type &handler)
	{	
		remote->read_handler_ = handler;
	}

	void register_write_handler(const session_ptr &remote, const write_handler_type &handler)
	{
		remote->write_handler_ = handler;
	}

	


	// ------------------------------------

	struct io_thread::impl
	{
		iocp::io_dispatcher io_;

		impl(){}
		impl(std::uint32_t num)
			: io_(num)
		{}
	};


	io_thread::io_thread()
		: impl_(new impl)
	{

	}

	io_thread::io_thread(std::uint32_t num)
		: impl_(new impl(num))
	{

	}

	io_thread::~io_thread()
	{

	}

	void io_thread::stop()
	{
		impl_->io_.stop();
	}

	struct client::impl
		: public std::enable_shared_from_this<client::impl>
	{
		io_thread &io_;
		network::tcp::socket socket_;
		timer::timer_handle_ptr timer_;

		client::error_handler_type error_handle_;
		client::connect_handler_type connect_handle_;
		client::disconnect_handler_type disconnect_handle_;
		client::read_handler_type read_handle_;
		client::write_handler_type write_handle_;

	public:
		impl(io_thread &io)
			: io_(io)
			, socket_(io_.impl_->io_, network::tcp::v4())
		{
		}
		~impl()
		{

		}

	public:
		bool async_send(const iocp::const_buffer &buffer)
		{
			try
			{
				iocp::async_write(socket_, buffer, iocp::transfer_all(), 
					std::bind(&impl::_handle_write, shared_from_this(), iocp::_Error, iocp::_Size));
			}
			catch(exception::exception_base &e)
			{
				e.dump();

				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));
				_disconnect();
				return false;
			}
			catch(std::exception &e)
			{
				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));
				_disconnect();
				return false;
			}

			return true;
		}

		bool async_recv(iocp::mutable_buffer &buffer)
		{
			try
			{
				iocp::async_read(socket_, buffer, iocp::transfer_all(), 
					std::bind(&impl::_handle_read, shared_from_this(), iocp::_Error, iocp::_Size));
			}
			catch(exception::exception_base &e)
			{
				e.dump();

				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));

				_disconnect();
				return false;
			}
			catch(std::exception &e)
			{
				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));

				_disconnect();
				return false;
			}

			return true;
		}

		std::uint32_t send(const iocp::const_buffer &buffer)
		{
			try
			{
				return iocp::write(socket_, buffer);
			}
			catch(exception::exception_base &e)
			{
				e.dump();

				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));

				_disconnect();
				return 0;
			}
			catch(std::exception &e)
			{
				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));

				_disconnect();
				return false;
			}
		}

		std::uint32_t recv(iocp::mutable_buffer &buffer)
		{
			try
			{
				return iocp::read(socket_, buffer);
			}
			catch(exception::exception_base &e)
			{
				e.dump();

				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));

				_disconnect();
				return 0;
			}
			catch(std::exception &e)
			{
				if( error_handle_ )
					error_handle_(std::cref(std::string(e.what())));

				_disconnect();
				return false;
			}
		}


		bool start(const std::string &ip, std::uint16_t port, time_second_type time_out)
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
				bool suc = socket_.set_option(network::recv_time_out(time_out * 1000));
				suc = socket_.set_option(network::send_time_out(time_out * 1000));

				timer_.reset(new timer::timer_handle(io_.impl_->io_));
				timer_->async_wait([this]()
				{ 
					_disconnect(); 
				}, time_out, time_out);

				socket_.connect(network::ip_address::parse(ip), port);
				_on_connect(async::iocp::win32_error_2_error_code(0));
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

		void stop()
		{
			socket_.close();
		}

		void _on_connect(iocp::error_code error)
		{
            try
            {
				if( connect_handle_ )
					connect_handle_(error == 0);

                if( error == 0 )
                {
					timer_->cancel();
				}
                else
                {
					if( error_handle_ )
						error_handle_(std::cref(error_msg(error)));
                    _disconnect();
                }
            }
			catch(...)
            {
				assert(0 && "has an exception in connect_handler_");
            }
		}

		void _handle_read(iocp::error_code error, u_long bytes)
		{
			try
			{
				if( error == 0 )
				{
					if( bytes == 0 )
						_disconnect();
					else if( read_handle_ )
						read_handle_(bytes);
				}
				else
				{
					if( error_handle_ )
						error_handle_(std::cref(error_msg(error)));
					_disconnect();
				}
			}
			catch(...)
			{
				assert(0 && "has an exception in read_handler_");
			}
		}

		void _handle_write(iocp::error_code error, u_long bytes)
		{
			if( error == 0 )
			{
				if( bytes == 0 )
					_disconnect();
				else if( write_handle_ )
					write_handle_(bytes);
			}
			else
			{
				if( error_handle_ )
					error_handle_(std::cref(error_msg(error)));
				_disconnect();
			}
		}

		void _disconnect()
		{
			timer_->cancel();

            try
            {
				if( disconnect_handle_ )
					disconnect_handle_();
            }
			catch(...)
            {
                assert(0 && "has an exception in disconnect_handler_");
            }
			stop();
		}
	};


	client::client(io_thread &io)
		: impl_(new impl(io))
	{

	}

	client::~client()
	{

	}

	void client::register_error_handler(const error_handler_type &handler)
	{
		impl_->error_handle_ = handler;
	}

	void client::register_connect_handler(const connect_handler_type &handler)
	{
		impl_->connect_handle_ = handler;
	}

	void client::register_disconnect_handler(const disconnect_handler_type &handler)
	{
		impl_->disconnect_handle_ = handler;
	}

	void client::register_read_handler(const read_handler_type &handler)
	{	
		impl_->read_handle_ = handler;
	}

	void client::register_write_handler(const write_handler_type &handler)
	{
		impl_->write_handle_ = handler;
	}


	bool client::start(const std::string &ip, u_short port, time_second_type time_out /* = 0 */)
	{
		return impl_->start(ip, port, time_out);
	}

	void client::stop()
	{
		impl_->stop();
	}

	bool client::async_send(const char *buf, std::uint32_t len)
	{
		return impl_->async_send(iocp::buffer(buf, len));
	}

	bool client::async_recv(char *buf, std::uint32_t len)
	{
		return impl_->async_recv(iocp::buffer(buf, len));
	}

	std::uint32_t client::send(const char *buf, std::uint32_t len)
	{
		return impl_->send(iocp::buffer(buf, len));
	}

	std::uint32_t client::recv(char *buf, std::uint32_t len)
	{
		return impl_->recv(iocp::buffer(buf, len));
	}
}
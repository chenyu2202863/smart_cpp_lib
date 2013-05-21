#ifndef __ASYNC_NETWORK_HPP
#define __ASYNC_NETWORK_HPP

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <chrono>

#include "service/dispatcher.hpp"
#include "service/read_write_buffer.hpp"
#include "network/tcp.hpp"
#include "timer/timer.hpp"


namespace async { namespace network { 

	class session;
	typedef std::shared_ptr<session> session_ptr;
	typedef std::weak_ptr<session> session_weak_ptr;

	typedef std::function<void(const session_ptr &, const std::string &msg)>	error_handler_type;
	typedef std::function<bool(const session_ptr &, const std::string &ip)>		accept_handler_type;
	typedef std::function<void(const session_ptr &)>							disconnect_handler_type;

	std::string error_msg(std::error_code err);

	// ----------------------------------

	class session
		: public std::enable_shared_from_this<session>
	{
		service::io_dispatcher_t &io_;
		tcp::socket sck_;

		void *data_;

	public:
		const error_handler_type &error_handler_;
		const disconnect_handler_type &disconnect_handler_;

	public:
		session(service::io_dispatcher_t &io, socket_handle_t &&sck, 
			const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler);
		~session();

	private:
		session(const session &);
		session &operator=(const session &);

	public:
		const tcp::socket &get() const { return sck_; }
		std::string get_ip() const;

		template < typename BufferT, typename HandlerT >
		bool async_read(BufferT &&buffer, HandlerT &&read_handler);

		template < typename BufferT, typename HandlerT >
		bool async_write(BufferT &&buffer, HandlerT &&write_handler);

		void additional_data(void *data);
		void *additional_data() const;

		void disconnect();

	private:
		template < typename HandlerT >
		void _handle_read(std::error_code error, std::uint32_t size, const HandlerT &read_handler);

		template < typename HandlerT >
		void _handle_write(std::error_code error, std::uint32_t size, const HandlerT &write_handler);
	};

	template < typename HandlerT >
	bool async_read(const session_ptr &val, char *buf, std::uint32_t len, HandlerT &&handler)
	{
		if( !val )
			return false;

		return val->async_read(std::move(async::service::buffer(buf, len)), std::forward<HandlerT>(handler));
	}

	template < typename HandlerT >
	bool async_write(const session_ptr &val, const char *buf, std::uint32_t len, HandlerT &&handler)
	{
		if( !val )
			return false;

		return val->async_write(std::move(async::service::buffer(buf, len)), std::forward<HandlerT>(handler));
	}

	template < typename HandlerT >
	bool async_read(const session_ptr &remote, async::service::mutable_array_buffer_t &&buf, HandlerT &&handler)
	{
		if( !remote )
			return false;

		return remote->async_read(buf, std::forward<HandlerT>(handler));
	}

	template < typename HandlerT >
	bool async_write(const session_ptr &remote, async::service::const_array_buffer_t &&buf, HandlerT &&handler)
	{
		if( !remote )
			return false;

		return remote->async_write(buf, std::forward<HandlerT>(handler));
	}

	// fix MS BUG
	template < typename HandlerT >
	struct handler_t
	{
		session_ptr session_; 
		HandlerT handler_;
		const error_handler_type &error_handler_;

		handler_t(session_ptr &&val, const error_handler_type &error_handler, HandlerT &&handler)
			: session_(std::move(val))
			, error_handler_(error_handler)
			, handler_(std::move(handler))
		{}
		handler_t(handler_t &&rhs)
			: session_(std::move(rhs.session_))
			, error_handler_(rhs.error_handler_)
			, handler_(std::move(rhs.handler_))
		{}

		void operator()(std::error_code error, std::uint32_t size)
		{
			if( error.value() == 0 )	// success
			{
				if( size == 0 )
				{
					// disconnect
					session_->disconnect();
				}
				else
					handler_(std::cref(session_), size);
			}
			else
			{
				if( error_handler_ )
				{
					error_handler_(std::cref(session_), std::cref(error_msg(error)));
				}
				session_->disconnect();
			}
		}
	};

	template < typename BufferT, typename HandlerT >
	bool session::async_read(BufferT &&buffer, HandlerT &&read_handler)
	{
		try
		{
			service::async_read(
				sck_, 
				buffer, 
				service::transfer_all(), 
				std::move(handler_t<typename std::remove_all_extents<HandlerT>::type>(shared_from_this(), error_handler_, std::forward<HandlerT>(read_handler))));

			// MS BUG!!
			//std::bind(&session::_handle_read<HandlerT>, shared_from_this(), async::service::_Error, async::service::_Size, std::move(read_handler)));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));

			disconnect();

			return false;
		}
		catch(std::exception &e)
		{
			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));

			disconnect();
			return false;
		}

		return true;
	}

	template < typename BufferT, typename HandlerT >
	bool session::async_write(BufferT &&buffer, HandlerT &&write_handler)
	{
		try
		{
			service::async_write(
				sck_, 
				buffer, 
				service::transfer_all(), 
				std::move(handler_t<typename std::remove_all_extents<HandlerT>::type>(shared_from_this(), error_handler_, std::forward<HandlerT>(write_handler))));
			//std::bind(&session::_handle_write<HandlerT>, shared_from_this(), async::service::_Error, async::service::_Size, std::move(write_handler)));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));
			disconnect();

			return false;
		}
		catch(std::exception &e)
		{
			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));

			disconnect();
			return false;
		}

		return true;
	}

	// ----------------------------------

	class server
	{

	private:	
		struct impl ;
		std::unique_ptr<impl> impl_;

	public:
		explicit server(std::uint16_t port);
		~server();

	private:
		server(const server &);
		server &operator=(const server &);

	public:
		bool start();
		bool stop();

		void register_error_handler(const error_handler_type &);
		void register_accept_handler(const accept_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);
	};


	// ----------------------------------

	class client
	{
	public:
		enum { DEFAULT_TIME_OUT = 3 };

	public:
		typedef std::function<void(const std::string &msg)>		error_handler_type;
		typedef std::function<void(bool)>						connect_handler_type;
		typedef std::function<void()>							disconnect_handler_type;

	private:	
		service::io_dispatcher_t &io_;
		tcp::socket socket_;

		timer::win_timer_service_t &timer_svr_;
		timer::timer_handle_ptr timer_;

		client::error_handler_type error_handle_;
		client::connect_handler_type connect_handle_;
		client::disconnect_handler_type disconnect_handle_;


	public:
		explicit client(service::io_dispatcher_t &io, timer::win_timer_service_t &timer_svr);
		~client();

	private:
		client(const client &);
		client &operator=(const client &);

	public:
		bool start(const std::string &ip, std::uint16_t port, std::chrono::seconds time_out);
		void stop();

		void register_error_handler(const error_handler_type &);
		void register_connect_handler(const connect_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);

	public:
		template < typename BufferT, typename HandlerT >
		bool async_send(BufferT &&buf, HandlerT &&handler);

		template < typename HandlerT >
		bool async_recv(char *buf, std::uint32_t len, HandlerT &&handler);

		std::uint32_t send(const char *buf, std::uint32_t len);
		std::uint32_t recv(char *buf, std::uint32_t len);

		void disconnect();

	private:
		void _on_connect(std::error_code error);
		
	};


	// fix MS BUG
	template < typename HandlerT >
	struct handler_wrapper_t
	{
		client &session_; 
		HandlerT handler_;
		const client::error_handler_type &error_handler_;

		handler_wrapper_t(client &val, const client::error_handler_type &error_handler, HandlerT &&handler)
			: session_(val)
			, error_handler_(error_handler)
			, handler_(std::move(handler))
		{}
		handler_wrapper_t(handler_wrapper_t &&rhs)
			: session_(rhs.session_)
			, error_handler_(rhs.error_handler_)
			, handler_(std::move(rhs.handler_))
		{}

		void operator()(std::error_code error, std::uint32_t size)
		{
			if( error.value() == 0 )	// success
			{
				if( size == 0 )
				{
					// disconnect
					session_.disconnect();
				}
				else
					handler_(size);
			}
			else
			{
				if( error_handler_ )
				{
					error_handler_(std::cref(error_msg(error)));
				}
				session_.disconnect();
			}
		}
	};

	template < typename BufferT, typename HandlerT >
	bool client::async_send(BufferT &&buf, HandlerT &&handler)
	{
		try
		{
			service::async_write(socket_, std::forward<BufferT>(buf), service::transfer_all(),
				std::move(handler_wrapper_t<typename std::remove_all_extents<HandlerT>::type>(*this, error_handle_, std::forward<HandlerT>(handler))));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}

		return true;
	}

	template < typename HandlerT >
	bool client::async_recv(char *buf, std::uint32_t len, HandlerT &&handler)
	{
		try
		{
			service::async_read(socket_, async::service::buffer(buf, len), service::transfer_all(), 
				std::move(handler_wrapper_t<typename std::remove_all_extents<HandlerT>::type>(*this, error_handle_, std::forward<HandlerT>(handler))));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return false;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return false;
		}

		return true;
	}
}
}

#endif
#ifndef __ASYNC_NETWORK_HPP
#define __ASYNC_NETWORK_HPP

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <chrono>
#include <list>
#include <atomic>

#include "basic.hpp"
#include "service/read_write_buffer.hpp"
#include "service/multi_buffer.hpp"
#include "network/tcp.hpp"
#include "timer/timer.hpp"
#include "../extend_stl/type_traits.hpp"
#include "../utility/move_wrapper.hpp"



namespace async { namespace network { 

	class server;
	class session;
	typedef std::shared_ptr<session> session_ptr;
	typedef std::weak_ptr<session> session_weak_ptr;

	typedef std::function<void(const session_ptr &, const std::string &msg)>	error_handler_type;
	typedef std::function<bool(const session_ptr &, const ip_address &)>		accept_handler_type;
	typedef std::function<void(const session_ptr &)>							disconnect_handler_type;


	std::string error_msg(const std::error_code &err);

	// ----------------------------------

	class session
		: public std::enable_shared_from_this<session>
	{
		struct holder_t
		{
			virtual ~holder_t() {}
			virtual void *get() = 0;
		};

		template < typename T >
		struct holder_impl_t
			: holder_t
		{
			T val_;

			holder_impl_t(const T &val)
				: val_(val)
			{}

			virtual void *get() override
			{
				return &val_;
			}
		};

		server &svr_;
		std::atomic<bool> is_valid_ = true;
		std::shared_ptr<holder_t> data_;

	public:
		mutable std::shared_ptr<socket_handle_t> sck_;
		const error_handler_type &error_handler_;
		const disconnect_handler_type &disconnect_handler_;

	public:
		session(server &svr, std::shared_ptr<socket_handle_t> &&sck, 
			const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler);
		~session();

	private:
		session(const session &);
		session &operator=(const session &);

	public:
		bool is_valid() const
		{ return is_valid_; }
		std::string get_ip() const;

		socket_handle_t &native_handle()
		{ return *sck_; }

		template < typename T >
		bool set_option(const T &v)
		{
			return sck_->set_option(v);
		}

		template < typename T >
		bool io_control(T &v)
		{
			return sck_->io_control(v);
		}

		template < typename HandlerT, typename AllocatorT = std::allocator<char> >
		bool async_read(service::mutable_buffer_t &, HandlerT &&, const AllocatorT &allocator = AllocatorT());
		template < typename HandlerT, typename AllocatorT = std::allocator<char> >
		bool async_read_some(service::mutable_buffer_t &, std::uint32_t min_len, HandlerT &&, const AllocatorT &allocator = AllocatorT());

		template < typename HandlerT, typename AllocatorT = std::allocator<char> >
		bool async_write(const service::const_buffer_t &, HandlerT &&, const AllocatorT &allocator = AllocatorT());
		template < typename HandlerT, typename AllocatorT, typename ...Args >
		bool async_write(HandlerT &&, const AllocatorT &, Args &&...);

		template < typename T, typename AlocatorT >
		void additional_data(const T &t, const AlocatorT &allocator);

		template < typename T >
		T &additional_data();

		void shutdown();
		void disconnect();

		template < typename HandlerT >
		void _handle_read(const std::error_code &error, std::uint32_t size, const HandlerT &read_handler);

		template < typename HandlerT >
		void _handle_write(const std::error_code &error, std::uint32_t size, const HandlerT &write_handler);
	
		template < typename HandlerT >
		bool _run_impl(HandlerT &&handler, bool is_read_op);
	};

	template < typename HandlerT, typename AllocatorT = std::allocator<char> >
	bool async_read(const session_ptr &val, char *buf, std::uint32_t len, HandlerT &&handler, const AllocatorT &allocator = AllocatorT())
	{
		if( !val )
			return false;

		return val->async_read(std::move(async::service::buffer(buf, len)), std::forward<HandlerT>(handler), allocator);
	}

	template < typename HandlerT, typename AllocatorT = std::allocator<char> >
	bool async_write(const session_ptr &val, const char *buf, std::uint32_t len, HandlerT &&handler, const AllocatorT &allocator = AllocatorT())
	{
		if( !val )
			return false;

		return val->async_write(std::move(async::service::buffer(buf, len)), std::forward<HandlerT>(handler), allocator);
	}


	template < typename HandlerT, typename AllocatorT >
	bool session::async_read(service::mutable_buffer_t &buffer, HandlerT &&read_handler, const AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			auto this_val = shared_from_this();
			auto handler_val = utility::make_move_obj(std::forward<HandlerT>(read_handler));

			service::async_read(*sck_,
				buffer,
				service::transfer_all(),
				[this_val, handler_val](const std::error_code &err, std::uint32_t len)
			{
				this_val->_handle_read(err, len, handler_val.value_);
			}, allocator);
		}, true);
	}

	template < typename HandlerT, typename AllocatorT >
	bool session::async_read_some(service::mutable_buffer_t &buffer, std::uint32_t min_len, HandlerT && handler, const AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			auto this_val = shared_from_this();
			auto handler_val = utility::make_move_obj(std::forward<HandlerT>(handler));

			sck_->async_read(buffer, 
				[this_val, handler_val](const std::error_code &err, std::uint32_t len)
			{
				this_val->_handle_read(err, len, handler_val.value_);
			}, allocator);
		}, true);
	}

	template < typename HandlerT, typename AllocatorT >
	bool session::async_write(const service::const_buffer_t &buffer, HandlerT &&write_handler, const AllocatorT &allocator)
	{
		if( !is_valid() )
			return false;

		return _run_impl([&]()
		{
			auto this_val = shared_from_this();
			auto handler_val = utility::make_move_obj(std::forward<HandlerT>(write_handler));

			service::async_write(
				*sck_, 
				buffer, 
				service::transfer_all(), 
				[this_val, handler_val](const std::error_code &err, std::uint32_t len) 
			{ 
				this_val->_handle_write(err, len, handler_val.value_);
			}, allocator);
		}, false);
	}

	template < typename ParamT >
	struct hook_handler_t
		: ParamT
	{
		session_ptr session_;

		hook_handler_t(session_ptr &&session, ParamT &&param)
			: session_(std::move(session))
			, ParamT(std::move(param))
		{}

		hook_handler_t(hook_handler_t &&rhs)
			: session_(std::move(rhs.session_))
			, ParamT(std::move(rhs))
		{}

		void operator()(const std::error_code &err, std::uint32_t len)
		{
			session_->_handle_write(err, len, *static_cast<ParamT *>(this));
		}
	};

	template < typename HandlerT, typename AllocatorT, typename ...Args >
	bool session::async_write(HandlerT &&handler, const AllocatorT &allocator, Args &&...args)
	{
		if( !is_valid() )
			return false;

		auto this_val = shared_from_this();
		auto param = service::make_param(std::forward<HandlerT>(handler), std::forward<Args>(args)...);

		typedef decltype(param) param_t;
		hook_handler_t<param_t> hook_handler(std::move(this_val), std::move(param));
		
		return _run_impl([&]()
		{
			sck_->async_write(std::move(hook_handler), allocator);
		}, false);
	}

	template < typename HandlerT >
	bool session::_run_impl(HandlerT && handler, bool is_read_op)
	{
		try
		{
			handler();
			return true;
		}
		catch (::exception::exception_base &e)
		{
			e.dump();

			if (error_handler_)
				error_handler_(shared_from_this(), std::string(e.what()));

			if( is_read_op )
				disconnect();

			return false;
		}
		catch (std::exception &e)
		{
			if (error_handler_)
				error_handler_(shared_from_this(), std::string(e.what()));

			if( is_read_op )
				disconnect();

			return false;
		}
	}

	template < typename HandlerT >
	void session::_handle_read(const std::error_code &error, std::uint32_t size, const HandlerT &read_handler)
	{
		try
		{
			if( !error )	// success
			{
				if( size == 0 )
				{
					// disconnect
					disconnect();
				}
				else
					read_handler(shared_from_this(), size);
			}
			else
			{
				if( size != 0 && error_handler_ )
				{
					error_handler_(shared_from_this(), error_msg(error));
				}

				disconnect();
			}
		}
		catch (...)
		{
			assert(0 && "has an exception in read_handler_");
			error_handler_(shared_from_this(), "has an exception in read_handler");
		}
	}

	template < typename HandlerT >
	void session::_handle_write(const std::error_code &error, std::uint32_t size, const HandlerT &write_handler)
	{
		try
		{
			if( error )	// error
			{
				if( error_handler_ )
					error_handler_(shared_from_this(), error_msg(error));

				size = 0;
			}

			write_handler(shared_from_this(), size);
		}
		catch (...)
		{
			assert(0 && "has an exception in write_handler_");
			error_handler_(shared_from_this(), "has an exception in write_handler_");
		}
	}

	template < typename T, typename AlocatorT >
	void session::additional_data(const T &t, const AlocatorT &allocator)
	{
		data_ = std::allocate_shared<holder_impl_t<T>>(allocator, t);
	}

	template < typename T >
	T &session::additional_data()
	{
		assert(data_ && "data is empty");
		void *val = data_->get();

		return *static_cast<T *>(val);
	}

	// ----------------------------------

	class server
	{
		friend class session;
	private:	
		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		explicit server(std::uint16_t port, std::uint32_t thr_cnt = 0);
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

		void post(std::function<void(const std::error_code &, std::uint32_t)> &&);
	};


	// ----------------------------------

	class client
	{
	public:
		typedef std::function<void(const std::string &msg)>		error_handler_type;
		typedef std::function<void(bool)>						connect_handler_type;
		typedef std::function<void()>							disconnect_handler_type;

	private:	
		service::io_dispatcher_t &io_;
		tcp::socket socket_;

		client::error_handler_type error_handle_;
		client::connect_handler_type connect_handle_;
		client::disconnect_handler_type disconnect_handle_;

	public:
		explicit client(service::io_dispatcher_t &io);
		~client();

	private:
		client(const client &);
		client &operator=(const client &);

	public:
		bool async_start(const std::string &ip, std::uint16_t port);
		bool start(const std::string &ip, std::uint16_t port);
		void stop();

		void register_error_handler(const error_handler_type &);
		void register_connect_handler(const connect_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);

		tcp::socket &native_handle()
		{ return socket_; }


		template < typename T >
		bool set_option(const T &v)
		{ return socket_.set_option(v); }

		template < typename T >
		bool io_control(T &val)
		{ return socket_.io_control(val); }

	public:
		template < typename HandlerT, typename AllocatorT = std::allocator<char> >
		bool async_send(const service::const_buffer_t &, HandlerT &&, const AllocatorT &allocator = AllocatorT());
		template < typename HandlerT, typename AllocatorT, typename ...Args >
		typename std::enable_if<stdex::is_callable<HandlerT(std::uint32_t)>::value, bool>::type async_send(HandlerT &&, const AllocatorT &, Args &&...);

		template < typename HandlerT, typename AllocatorT = std::allocator<char> >
		bool async_recv(char *, std::uint32_t, HandlerT &&, const AllocatorT &allocator = AllocatorT());
		template < typename HandlerT, typename AllocatorT = std::allocator<char> >
		bool async_read_some(service::mutable_buffer_t &, std::uint32_t min_len, HandlerT &&, const AllocatorT &allocator = AllocatorT());


		bool send(const char *buf, std::uint32_t len);
		bool recv(char *buf, std::uint32_t len);
		std::uint32_t read_some(char *buf, std::uint32_t len);

		void disconnect();

	private:
		template < typename HandlerT >
		bool _run_impl(HandlerT && handler);

		void _on_connect(const std::error_code &);
		
	};


	// fix MS BUG
	template < typename HandlerT >
	struct cli_handler_wrapper_t
		: HandlerT
	{
		typedef cli_handler_wrapper_t<HandlerT> this_type;

		client &session_; 
		const client::error_handler_type &error_handler_;

		cli_handler_wrapper_t(client &val, const client::error_handler_type &error_handler, HandlerT &&handler)
			: HandlerT(std::move(handler))
			, session_(val)
			, error_handler_(error_handler)
		{}
		cli_handler_wrapper_t(cli_handler_wrapper_t &&rhs)
			: HandlerT(std::move(rhs))
			, session_(rhs.session_)
			, error_handler_(rhs.error_handler_)
			
		{}

		void operator()(const std::error_code &error, std::uint32_t size)
		{
			if( !error )	// success
			{
				if( size == 0 )
				{
					// disconnect
					session_.disconnect();
				}
				else
					static_cast<HandlerT *>(this)->operator()(size);
			}
			else
			{
				if( size != 0 && error_handler_ )
				{
					error_handler_(error_msg(error));
				}
				session_.disconnect();
			}
		}
	};

	template < typename HandlerT, typename AllocatorT >
	bool client::async_send(const service::const_buffer_t &buf, HandlerT &&handler, const AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			service::async_write(socket_, buf, service::transfer_all(),
				std::move(cli_handler_wrapper_t<HandlerT>(*this, error_handle_, std::forward<HandlerT>(handler))), allocator);
		});
	}


	template < typename HandlerT, typename AllocatorT, typename ...Args >
	typename std::enable_if<stdex::is_callable<HandlerT(std::uint32_t)>::value, bool>::type client::async_send(HandlerT &&handler, const AllocatorT &allocator, Args &&...args)
	{
		return _run_impl([&]()
		{
			auto handler_val = service::make_param(std::forward<HandlerT>(handler), std::forward<Args>(args)...);
			return socket_.async_write(cli_handler_wrapper_t<decltype(handler_val)>(*this, 
					error_handle_, 
					std::move(handler_val)), allocator);
		});
	}

	template < typename HandlerT, typename AllocatorT  >
	bool client::async_recv(char *buf, std::uint32_t len, HandlerT &&handler, const AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			service::async_read(socket_, async::service::buffer(buf, len), service::transfer_all(), 
				cli_handler_wrapper_t<HandlerT>(*this, error_handle_, std::forward<HandlerT>(handler)), allocator);
		});
	}

	template < typename HandlerT, typename AllocatorT  >
	bool client::async_read_some(service::mutable_buffer_t &buffer, std::uint32_t min_len, HandlerT && handler, const AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			socket_.async_read(buffer, 
				cli_handler_wrapper_t<HandlerT>(*this, error_handle_, std::forward<HandlerT>(handler)), 
				allocator);
		});
	}

	template < typename HandlerT >
	bool client::_run_impl(HandlerT && handler)
	{
		try
		{
			handler();
			return true;
		}
		catch(::exception::exception_base &e )
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::string(e.what()));

			disconnect();
			return false;
		}
		catch( std::exception &e )
		{
			if( error_handle_ )
				error_handle_(std::string(e.what()));

			disconnect();
			return false;
		}
	}
}
}

#endif
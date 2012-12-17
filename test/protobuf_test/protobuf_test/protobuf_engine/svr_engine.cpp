#include "svr_engine.hpp"

#include <unordered_map>
#include <iostream>

#include "network.hpp"
#include "async_io/iocp/object_factory.hpp"
#include "extend_stl/container/sync_container.hpp"
#include "extend_stl/allocator/allocators.hpp"

#include "engine_base.hpp"
#include "codec.hpp"





namespace proto_engine
{

	class session
		: public std::enable_shared_from_this<session>
	{
		bool is_header_;
		wx::session_ptr remote_;
		std::string recv_buffer_;
		std::string send_buffer_;
		
		codec_t &codec_;
		const wx::error_handler_type &error_handler_;

	public:
		session(const wx::session_ptr &remote, codec_t &codec, const wx::error_handler_type &error_handler)
			: is_header_(true)
			, remote_(remote)
			, codec_(codec)
			, error_handler_(error_handler)
		{}

		~session()
		{

		}

		bool start()
		{
			is_header_ = true;
			recv_buffer_.resize(sizeof(std::uint32_t));
			bool suc = wx::async_read(remote_, &recv_buffer_[0], sizeof(std::uint32_t));
			assert(suc && "async_read");

			return suc;
		}

		void stop()
		{
			wx::close(remote_);
		}

		bool send(const google::protobuf::Message &msg)
		{
			send_buffer_.clear();
			codec_t::fill_buffer(send_buffer_, msg);

			return wx::async_write(remote_, send_buffer_.data(), send_buffer_.length());
		}

		void handle_read(std::uint32_t len)
		{
			if( is_header_ )
			{
				is_header_ = false;

				const std::uint32_t header_len = *reinterpret_cast<const std::uint32_t *>(recv_buffer_.data());
				const std::uint32_t total_len = header_len + len;

				if( len != sizeof(std::uint32_t) )
				{
					assert(0);
				}
				
				recv_buffer_.resize(total_len);
				bool suc = wx::async_read(remote_, &recv_buffer_[len], header_len);
				assert(suc && "async_read error");
			}
			else
			{
				if( len != recv_buffer_.size() - sizeof(std::uint32_t) )
				{
					assert(0);
				}

				is_header_ = true;
				codec_.on_msg(shared_from_this(), recv_buffer_.data(), recv_buffer_.size());
			}
		}

		void handle_write(std::uint32_t len)
		{
			is_header_ = true;
			recv_buffer_.clear();
			recv_buffer_.resize(sizeof(std::uint32_t));
			wx::async_read(remote_, &recv_buffer_[0], sizeof(std::uint32_t));
		}
	};


	bool send(const session_ptr &remote, const google::protobuf::Message &msg)
	{
		bool suc = remote->send(msg);
		assert(suc && "send message sucess");
		return suc;
	}
}

// 定制自己的工厂
namespace async
{
	namespace iocp
	{
		template<>
		struct object_factory_t< proto_engine::session >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(proto_engine::session)>	PoolType;
			typedef object_pool_t< PoolType >												ObjectPoolType;
		};
	}
}



namespace proto_engine
{
	// to do shared_ptr allocator

	inline session_ptr create_session(const wx::session_ptr &remote, codec_t &codec, 
		const std::function<void(const wx::session_ptr &, const std::string &)> &error_handler)
	{
		return session_ptr(async::iocp::object_allocate<session>(remote, codec, error_handler)
			, &async::iocp::object_deallocate<session>);
	}

	using namespace std::placeholders;

	namespace detail
	{
		void on_default_msg(session_ptr &remote, const msg_ptr &msg)
		{
			assert(0 && "remote socket error");
			remote->stop();
		}
	}

	struct server::impl
	{
	private:
		wx::server svr_;
		codec_t codec_;
		error_handler_type error_handler_;
		accept_handler_type accept_handler_;
		disconnect_handler_type disconnect_handler_;

		typedef std::unordered_map<wx::session_ptr, session_ptr> sessions_type;
		typedef stdex::container::sync_assoc_container_t<wx::session_ptr, session_ptr, sessions_type> sessions_container;
		sessions_container sessions_;

	public:
		impl(std::uint16_t port, dispatcher_t &dispatcher, const error_handler_type &error_handler)
			: svr_(port)
			, codec_(std::bind(&dispatcher_t::on_protobuf_msg, std::ref(dispatcher), _1, _2), error_handler)
			, error_handler_(error_handler)
		{
			assert(error_handler_ != 0);

			engine_base::instance();
			svr_.register_error_handler(std::bind(&impl::_handle_error, this, _1, _2));
		}

		~impl()
		{
		}

	public:
		bool start()
		{
			return _start_impl();
		}

		void stop()
		{
			_stop_impl();
		}

		void register_session(const accept_handler_type &connect, const disconnect_handler_type &disconnect)
		{
			accept_handler_ = connect;
			disconnect_handler_ = disconnect;

			svr_.register_accept_handler(std::bind(&impl::_handle_accept, this, _1, _2));
			svr_.register_disconnect_handler(std::bind(&impl::_handle_disconnect, this, _1));
		}

	private:
		bool _start_impl()
		{		
			return svr_.start();
		}

		void _stop_impl()
		{
			svr_.stop();
		}

		void _handle_error(const wx::session_ptr &remote, const std::string &msg)
		{
			sessions_.op_if(remote, 
				[this, &msg](const sessions_container::value_type &val)
			{ 
				error_handler_(std::cref(val.second), std::cref(msg));
			});
		}

		bool _handle_accept(const wx::session_ptr &remote, const std::string &ip)
		{
			session_ptr val(create_session(remote, codec_, std::bind(&impl::_handle_error, this, _1, _2)));
			wx::register_read_handler(remote, std::bind(&session::handle_read, val, _2));
			wx::register_write_handler(remote, std::bind(&session::handle_write, val, _2));

			sessions_.insert(remote, val);
			accept_handler_(std::cref(val));

			return val->start();
		}

		void _handle_disconnect(const wx::session_ptr &remote)
		{
			sessions_.op_if(remote, 
				[this](const sessions_container::value_type &val)
			{ 
				val.second->stop();
				disconnect_handler_(std::cref(val.second));
			});
			
			sessions_.erase(remote);
		}

		/*void _handle_read(const wx::session_ptr &remote, std::uint32_t len)
		{
			sessions_.op_if(remote, 
				[len](const sessions_container::value_type &val)
			{ 
				val.second->handle_read(len); 
			});
		}

		void _handle_write(const wx::session_ptr &remote, std::uint32_t len)
		{
			sessions_.op_if(remote, 
				[len](const sessions_container::value_type &val)
			{ 
				val.second->handle_write(len); 
			});
		}*/
	};


	server::server(std::uint16_t port, const error_handler_type &error_handler)
		: dispatcher_(std::bind(&detail::on_default_msg, _1, _2))
		, impl_(new impl(port, dispatcher_, error_handler))
	{

	}

	server::~server()
	{

	}

	bool server::start()
	{
		return impl_->start();
	}

	void server::stop()
	{
		impl_->stop();
	}

	void server::register_session(const accept_handler_type &connect, const disconnect_handler_type &disconnect)
	{
		impl_->register_session(connect, disconnect);
	}
}
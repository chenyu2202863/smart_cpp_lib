#include "cli_engine.hpp"

#include "network.hpp"
#include "engine_base.hpp"
#include "codec.hpp"


namespace proto_engine
{
	using namespace std::placeholders;


	wx::io_thread io_(1);

	struct client::impl
	{
	private:
		wx::client cli_;
		codec_t<wx::client> codec_;
		std::string recv_buffer_;
		std::string send_buffer_;

		error_handler_type error_handler_;

	public:
		impl(const error_handler_type &error_handler)
			: cli_(io_)
			, codec_([](wx::client &, const proto_engine::msg_ptr &){}, 
				[this](wx::client &, const std::string &msg)
				{
					error_handler_(std::cref(msg));
				})
			, error_handler_(error_handler)
		{
			engine_base::instance();
		}
		~impl()
		{
		}

	public:
		bool start(const std::string &ip, u_short port)
		{
			cli_.register_error_handler(error_handler_);
			cli_.register_disconnect_handler(std::bind(&impl::stop, this));

			return cli_.start(ip, port, 10);
		}

		void stop()
		{
			cli_.stop();
		}

		std::pair<bool, proto_engine::msg_ptr> send(const google::protobuf::Message &msg)
		{
			send_buffer_.clear();
			codec_t<wx::client>::fill_buffer(send_buffer_, msg);

			if( !send_buffer_.empty() )
			{
				bool suc = cli_.async_send(send_buffer_.data(), send_buffer_.size());
				assert(suc);
				/*size_t ret = cli_.async_send(buffer_.data(), buffer_.size());
				if( ret == 0 )
				{
					error_handler_("发送数据失败");
					return std::make_pair(false, proto_engine::msg_ptr());
				}*/
			}

			recv_buffer_.clear();
			recv_buffer_.resize(sizeof(std::uint32_t));
			size_t len = cli_.recv(&recv_buffer_[0], sizeof(std::uint32_t));
			if( len != sizeof(std::uint32_t) )
			{
				error_handler_("接收数据包头失败");
				return std::make_pair(false, proto_engine::msg_ptr());
			}

			std::uint32_t msg_len = *reinterpret_cast<const std::uint32_t *>(recv_buffer_.data());
			recv_buffer_.resize(sizeof(std::uint32_t) + msg_len);
			size_t ret = cli_.recv(&recv_buffer_[sizeof(std::uint32_t)], msg_len);
			if( ret == 0 || ret != msg_len )
			{
				error_handler_("接收数据包体失败");
				return std::make_pair(false, proto_engine::msg_ptr());
			}
					
			error_code err;
			proto_engine::msg_ptr ret_msg = codec_.parse(recv_buffer_.data() + sizeof(std::uint32_t), msg_len, err);
			if( ret_msg == 0 )
			{
				error_handler_(std::cref(error_code_2_string(err)));
				return std::make_pair(false, ret_msg);
			}

			return std::make_pair(true, ret_msg);
		}
	};



	client::client(const error_handler_type &error_handler)
		: impl_(new impl(error_handler))
	{

	}
	client::~client()
	{

	}

	bool client::start(const std::string &ip, std::uint16_t port)
	{
		return impl_->start(ip, port);
	}

	void client::stop()
	{
		impl_->stop();
	}

	std::pair<bool, proto_engine::msg_ptr> client::send_impl(const google::protobuf::Message &msg)
	{
		return impl_->send(msg);
	}
}
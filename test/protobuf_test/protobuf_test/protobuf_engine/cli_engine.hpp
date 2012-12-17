#ifndef __CLIENT_DISPATCH_ENGINE_HPP
#define __CLIENT_DISPATCH_ENGINE_HPP

#include <functional>
#include <cstdint>
#include "base.hpp"

namespace proto_engine
{

	
	class client
	{
		typedef std::function<void(const std::string &)> error_handler_type;

	private:
		struct impl;
		std::auto_ptr<impl> impl_;
		
	public:
		client(const error_handler_type &error_handler);
		~client();

	private:
		client(const client &);
		client &operator=(const client &);

	public:
		bool start(const std::string &ip, std::uint16_t port);
		void stop();
		
		template < typename MsgT >
		std::pair<bool, std::shared_ptr<MsgT>> send(const google::protobuf::Message &msg)
		{
			std::pair<bool, proto_engine::msg_ptr> ret = send_impl(msg);

			std::shared_ptr<MsgT> val = std::dynamic_pointer_cast<MsgT>(ret.second);
			assert(val != 0);

			return std::make_pair(ret.first, val);
		}

	private:
		std::pair<bool, proto_engine::msg_ptr> send_impl(const google::protobuf::Message &msg);
	};

}




#endif
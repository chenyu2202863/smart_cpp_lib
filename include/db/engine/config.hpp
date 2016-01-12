#ifndef __DB_PROTOBUF_CONFIG_HPP
#define __DB_PROTOBUF_CONFIG_HPP

#include <cstdint>
#include <functional>
#include <memory>



namespace google { namespace protobuf {

	class Message;
}}

namespace gpb = google::protobuf;


namespace async { namespace proto {

	typedef std::function<void(const std::string &)>	error_handler_t;

	typedef std::shared_ptr<google::protobuf::Message>	msg_ptr;

}}

#endif
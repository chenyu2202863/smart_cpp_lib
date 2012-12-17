#ifndef __PROTOBUF_ENGINE_BASE_HPP
#define __PROTOBUF_ENGINE_BASE_HPP

#include <memory>

#include "third_party/google/protobuf/message.h"
#include "third_party/google/protobuf/descriptor.h"


namespace proto_engine
{
	typedef std::shared_ptr<google::protobuf::Message> msg_ptr;
}

#endif
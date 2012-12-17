#ifndef __PROTO_ENGINE_BASE_HPP
#define __PROTO_ENGINE_BASE_HPP

#include <memory>
#include "third_party/google/protobuf/message.h"

namespace proto_engine
{

	typedef std::shared_ptr<google::protobuf::Message> msg_ptr;

	struct engine_base
	{
	private:
		engine_base()
		{
			GOOGLE_PROTOBUF_VERIFY_VERSION;
		}

		~engine_base()
		{
			google::protobuf::ShutdownProtobufLibrary();
		}

	public:
		static engine_base &instance()
		{
			static engine_base engine;
			return engine;
		}
	};
}







#endif
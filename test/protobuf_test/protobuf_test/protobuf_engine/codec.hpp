#ifndef __PROTOBUF_ENGINE_CODEC_HPP
#define __PROTOBUF_ENGINE_CODEC_HPP

#include <cstdint>
#include <functional>

#include "base.hpp"



namespace async
{
	namespace iocp
	{
		class const_buffer;
	}
}



namespace proto_engine
{
	
	
	class session;
	typedef std::shared_ptr<session> session_ptr;

	enum error_code;

	// ----------------------------

	class codec_t
	{
		const static int kHeaderLen = sizeof(std::uint32_t);
		const static int kMinMessageLen = 2 * kHeaderLen + 2;	// nameLen + typeName + checkSum
		const static int kMaxMessageLen = 64 * 1024 * 1024;		// same as codec_stream.h kDefaultTotalBytesLimit

	public:
		typedef std::function<void (session_ptr&, const msg_ptr&)>			msg_callback_type;
		typedef std::function<void (session_ptr&, const std::string &)>		error_callback_type;

	private:
		struct impl;
		std::auto_ptr<impl> impl_;

	public:
		codec_t(const msg_callback_type &msg_handler, const error_callback_type &error_handler);
		~codec_t();

	private:
		codec_t(const codec_t &);
		codec_t &operator=(const codec_t &);

	public:
		void on_msg(session_ptr& impl, const char *buf, std::uint32_t len);

	public:
		static void fill_buffer(std::string &buf, const google::protobuf::Message& message);
		static google::protobuf::Message* create_msg(const std::string& typeName);
		static msg_ptr parse(const char* buf, int len, error_code &error);

	private:
		void _on_msg_impl(session_ptr& impl, const async::iocp::const_buffer &buf);

	};

}

#endif  
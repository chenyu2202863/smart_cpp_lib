#include "codec.hpp"

#include "extend_stl/allocator/allocators.hpp"
#include "algorithm/adler32.hpp"
#include "async_io/iocp/buffer.hpp"
#include "async_io/iocp/dispatcher.hpp"

// struct ProtobufTransportFormat __attribute__ ((__packed__))
// {
//   int32_t  len;
//   int32_t  nameLen;
//   char     typeName[nameLen];
//   char     protobufData[len-nameLen-8];
//   int32_t  checkSum; // adler32 of nameLen, typeName and protobufData
// }

namespace proto_engine
{

	namespace detail
	{
		inline std::int32_t AsInt32(const char* buf)
		{
			std::int32_t be32 = 0;
			::memcpy(&be32, buf, sizeof(be32));
			return be32;
		}


		const std::string kNoErrorStr		= "NoError";
		const std::string kInvalidLengthStr	= "InvalidLength";
		const std::string kCheckSumErrorStr	= "CheckSumError";
		const std::string kInvalidNameLenStr= "InvalidNameLen";
		const std::string kUnknownMessageTypeStr = "UnknownMessageType";
		const std::string kParseErrorStr	= "ParseError";
		const std::string kUnknownErrorStr	= "UnknownError";

	}

	enum error_code
	{
		kNoError = 0,
		kInvalidLength,
		kCheckSumError,
		kInvalidNameLen,
		kUnknownMessageType,
		kParseError,
	};

	const std::string &error_code_2_string(error_code errorCode)
	{
		switch (errorCode)
		{
		case kNoError:
			return detail::kNoErrorStr;
		case kInvalidLength:
			return detail::kInvalidLengthStr;
		case kCheckSumError:
			return detail::kCheckSumErrorStr;
		case kInvalidNameLen:
			return detail::kInvalidNameLenStr;
		case kUnknownMessageType:
			return detail::kUnknownMessageTypeStr;
		case kParseError:
			return detail::kParseErrorStr;
		default:
			return detail::kUnknownErrorStr;
		}
	}



	struct codec_t::impl
	{
		msg_callback_type msg_handler_;
		error_callback_type error_handler_;
		async::iocp::io_dispatcher io_;

		impl(const msg_callback_type &msg_handler, const error_callback_type &error_handler)
			: msg_handler_(msg_handler)
			, error_handler_(error_handler)
		{

		}
	};



	codec_t::codec_t(const msg_callback_type &msg_handler, const error_callback_type &error_handler)
		: impl_(new impl(msg_handler, error_handler))
	{
	}

	codec_t::~codec_t()
	{

	}


	void codec_t::on_msg(session_ptr& impl, const char *buf, std::uint32_t len)
	{
		impl_->io_.post(std::bind(&codec_t::_on_msg_impl, this, impl, async::iocp::const_buffer(buf, len)));

	}


	void codec_t::fill_buffer(std::string &buf, const google::protobuf::Message& message)
	{
		std::uint32_t total_len = 0;
		buf.append(reinterpret_cast<char *>(&total_len), sizeof(std::uint32_t));

		const std::string &typeName = message.GetTypeName();
		std::uint32_t nameLen = static_cast<std::uint32_t>(typeName.size()+1);
		buf.append(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
		buf.append(typeName.c_str(), nameLen);
		bool succeed = message.AppendToString(&buf);

		if( succeed )
		{
			const char* begin = reinterpret_cast<const char *>(buf.c_str() + kHeaderLen);
			std::uint32_t checkSum = algorithm::adler::adler32(begin, buf.size() - 2 * kHeaderLen);
			buf.append(reinterpret_cast<char*>(&checkSum), sizeof(checkSum));

			total_len = buf.size() - kHeaderLen;
			std::copy(reinterpret_cast<char*>(&total_len),
				reinterpret_cast<char*>(&total_len) + sizeof(total_len),
				buf.begin());
		}
		else
		{
			assert("buf is empty" && 0);
			buf.clear();
		}
	}



	google::protobuf::Message* codec_t::create_msg(const std::string& typeName)
	{
		google::protobuf::Message* message = nullptr;
		const google::protobuf::Descriptor* descriptor =
			google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
		if( descriptor )
		{
			const google::protobuf::Message* prototype =
				google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
			if( prototype )
			{
				message = prototype->New();
			}
		}
		return message;
	}

	msg_ptr codec_t::parse(const char* buf, int len, error_code &error)
	{
		std::int32_t expectedCheckSum = detail::AsInt32(buf + len - kHeaderLen);
		const char* begin = reinterpret_cast<const char *>(buf);
		std::int32_t checkSum = algorithm::adler::adler32(begin, len - 2 * kHeaderLen);
		if( checkSum == expectedCheckSum )
		{
			std::int32_t nameLen = detail::AsInt32(buf);
			if( nameLen >= 2 && nameLen <= len - 2 * kHeaderLen )
			{
				std::string type_name(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1);

				msg_ptr msg(create_msg(type_name), [](google::protobuf::Message *p){ delete p;});

				if( msg )
				{
					// parse from buffer
					const char* data = buf + kHeaderLen + nameLen;
					std::int32_t dataLen = len - nameLen - 2 * kHeaderLen;

					if( msg->ParseFromArray(data, dataLen) )
					{
						error = kNoError;
						return msg;
					}
					else
					{
						error = kParseError;
					}
				}
				else
				{
					error = kUnknownMessageType;
				}
			}
			else
			{
				error = kInvalidNameLen;
			}
		}
		else
		{
			error = kCheckSumError;
		}

		assert("msg_ptr is empty" && 0);
		return msg_ptr();
	}

	void codec_t::_on_msg_impl(session_ptr& impl, const async::iocp::const_buffer &buf)
	{
		size_t total = buf.size();
		size_t readableSize = total;

		while( readableSize >= kMinMessageLen + kHeaderLen )
		{
			const std::uint32_t len = *(std::uint32_t*)((buf + (total - readableSize)).data());
			if( len > kMaxMessageLen || len < kMinMessageLen )
			{
				assert("len > kMaxMessageLen || len < kMinMessageLen" && 0);
				impl_->error_handler_(std::ref(impl), std::cref(error_code_2_string(kInvalidLength)));
				break;
			}
			else if( readableSize >= google::protobuf::implicit_cast<size_t>(len + kHeaderLen))
			{
				error_code errorCode = kNoError;

				msg_ptr message = parse((buf + (total - readableSize + kHeaderLen)).data(), len, errorCode);
				if( errorCode == kNoError && message )
				{
					impl_->msg_handler_(std::ref(impl), std::ref(message));
					readableSize -= (len + kHeaderLen);
				}
				else
				{
					assert("errorCode == kNoError && message" && 0);
					impl_->error_handler_(std::ref(impl), std::cref(error_code_2_string(errorCode)));
					break;
				}
			}
			else
			{
				assert("read size error" && 0);
				break;
			}
		}
	}

}
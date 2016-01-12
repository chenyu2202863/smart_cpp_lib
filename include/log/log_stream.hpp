#ifndef __LOG_STREAM_HPP
#define __LOG_STREAM_HPP

#include <cstdint>
#include <functional>
#include <thread>

#include "../extend_stl/time.hpp"
#include "../serialize/serialize.hpp"

namespace logging {


	enum class level 
	{
		LEVEL_TRACE,
		LEVEL_DEBUG,
		LEVEL_INFO,
		LEVEL_WARN,
		LEVEL_ERROR,
		LEVEL_FATAL,
		LEVEL_NONE,
		NUM_LOG_LEVELS
	};

	extern const char* log_level_name[static_cast<const std::uint32_t>(level::NUM_LOG_LEVELS)];

	struct log_finisher_t;

	template < typename HandlerT >
	struct log_stream_t
	{
		const HandlerT &callback_;

		char buffer_[8192];
		serialize::o_text_serialize os_;
		const char *file_;
		std::uint32_t line_;

		friend struct log_finisher_t;


		log_stream_t(const HandlerT &callback, const char *file, std::uint32_t line, level l)
			: callback_(callback)
			, file_(file)
			, line_(line)
			, os_(buffer_)
		{
			std::time_t now{ 0 };
			std::time(&now);

			std::tm t_now = { 0 };
			::localtime_s(&t_now, &now);
			//std::strftime(time_buffer, _countof(time_buffer), "%Y-%m-%d %H:%M:%S", &t_now);

			// for performance
			os_ << 1900 + t_now.tm_year << '-'
				<< 1 + t_now.tm_mon << '-'
				<< t_now.tm_mday << ' '
				<< t_now.tm_hour << ':'
				<< t_now.tm_min << ':'
				<< t_now.tm_sec
				<< ' '
				<< log_level_name[static_cast<int>(l)];
		}

		log_stream_t(const HandlerT &callback, const char *file, std::uint32_t line)
			: callback_(callback)
			, file_(file)
			, line_(line)
			, os_(buffer_)
		{
		}

		log_stream_t(const log_stream_t &) = delete;
		log_stream_t &operator=(const log_stream_t &) = delete;

		template < typename T, typename ... Args >
		log_stream_t &print(const T &t, const Args &...args)
		{
			print(t);
			print(args...);
			return *this;
		}

		template < typename T >
		log_stream_t &print(const T &t)
		{
			os_ << t;
			return *this;
		}

		template < typename T >
		log_stream_t &operator<<(const T &t)
		{
			print(t);
			return *this;
		}

		void finish()
		{
			os_ << "\r\n";
			callback_({ buffer_, os_.in_length() });
		}
	};

	template < typename HandlerT >
	log_stream_t<HandlerT> make_log_stream(const HandlerT &handler, const char *file, std::uint32_t line, level l)
	{
		return log_stream_t<HandlerT>(handler, file, line, l);
	}


	struct empty_stream_t
	{
		template < typename T >
		empty_stream_t &operator<<(const T &t)
		{
			return *this;
		}
	};

	struct log_finisher_t
	{
		template < typename HandlerT >
		void operator=(log_stream_t<HandlerT> &stream)
		{
			stream.finish();
		}
	};
}

#endif
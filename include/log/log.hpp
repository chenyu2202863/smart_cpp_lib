#ifndef __LOGGER_HPP
#define __LOGGER_HPP

#include "log_stream.hpp"
#include "log_sink.hpp"

//#define LOG_DEBUG logging::log_stream_t os(__FILE__, __LINE__, logging::level::DEBUG); os.print

#define ASYNC_LOG(log_handler, LEVEL)	\
	logging::log_finisher_t() = \
	logging::log_stream_t<decltype(log_handler)>(log_handler, __FILE__, __LINE__, logging::level::LEVEL_##LEVEL)

#ifdef _DEBUG
#define LOG_TRACE(log_handler)	ASYNC_LOG(log_handler, TRACE)
#define LOG_DEBUG(log_handler)	ASYNC_LOG(log_handler, DEBUG)
#else
#define LOG_TRACE(log_handler)	logging::empty_stream_t()
#define LOG_DEBUG(log_handler)	logging::empty_stream_t()
#endif

#define LOG_INFO(log_handler)	ASYNC_LOG(log_handler, INFO)
#define LOG_WARN(log_handler)	ASYNC_LOG(log_handler, WARN)
#define LOG_ERROR(log_handler)	ASYNC_LOG(log_handler, ERROR)
#define LOG_FATAL(log_handler)	ASYNC_LOG(log_handler, FATAL)



#endif
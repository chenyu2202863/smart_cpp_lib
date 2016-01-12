#include "log_stream.hpp"

#include <Windows.h>


namespace logging {

	const char* log_level_name[static_cast<const std::uint32_t>(level::NUM_LOG_LEVELS)] =
	{
		"TRACE ",
		"DEBUG ",
		"INFO  ",
		"WARN  ",
		"ERROR ",
		"FATAL ",
		""
	};

	
	
}
#include "time.hpp"

#include <sstream>

#include <atltime.h>

namespace sys_time {

	std::chrono::system_clock::time_point to_time(const std::wstring &val)
	{
		std::wistringstream in(val);
		std::tm tm = { 0 };
		wchar_t delim = 0;
		in >> tm.tm_year >> delim >> tm.tm_mon >> delim >> tm.tm_mday >> delim >> tm.tm_hour;

		CTime time(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, 0, 0);
		return std::chrono::system_clock::from_time_t(time.GetTime());
	}


	std::wstring to_string(const std::chrono::time_point<std::chrono::system_clock> &time_point)
	{
		CTime time(std::chrono::system_clock::to_time_t(time_point));
		wchar_t buffer[32] = { 0 };
		wsprintf(buffer, L"%d-%02d-%02d %02d:%02d:%02d", time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

		return buffer;
	}
}
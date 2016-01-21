#ifndef __TIME_TIME_HPP
#define __TIME_TIME_HPP


#include <string>
#include <chrono>


namespace sys_time {

	std::chrono::system_clock::time_point to_time(const std::wstring &);

	std::wstring to_string(const std::chrono::time_point<std::chrono::system_clock> &);
}


#endif
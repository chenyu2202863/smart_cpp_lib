#ifndef __EXTEND_STL_STRING_HPP
#define __EXTEND_STL_STRING_HPP

#include "time/chrono.hpp"
#include "time/ratio.hpp"

#include <ctime>
#include <string>

namespace stdex
{
	namespace time
	{
		using namespace std::chrono;
		using std::ratio;


		// forward declare


		struct time_format;

		template < typename CharT >
		std::basic_string<CharT> time_2_string(time_t t, const CharT *format);

		template < typename CharT >
		std::basic_string<CharT> time_2_string(time_t t);

		template < typename CharT >
		time_t string_2_time(const std::basic_string<CharT> &time);

		template < typename CharT >
		time_t string_2_time(const std::basic_string<CharT> &time, const CharT *format);


		// ---------------------------------------------------
		// impl
		struct time_format
		{
			operator const char *() const
			{
				return "%Y-%m-%d %H:%M:%S";
			}

			operator const wchar_t *() const
			{
				return L"%Y-%m-%d %H:%M:%S";
			}
		};


		inline time_t now()
		{
			return system_clock::to_time_t(system_clock::now());
		}

		template < typename CharT >
		std::basic_string<CharT> time_2_string(time_t t, const CharT *format)
		{
			std::basic_ostringstream<CharT> time_buffer;
			tm tm_val = {0};
			::localtime_s(&tm_val, &t);

			time_buffer << std::put_time(&tm_val, format);

			return time_buffer.str();
		}

		template < typename CharT >
		std::basic_string<CharT> time_2_string(time_t t)
		{
			return time_2_string<CharT>(t, time_format());
		}

		template < typename CharT >
		time_t string_2_time(const std::basic_string<CharT> &time)
		{
			return string_2_time(time, time_format());
		}

		template < typename CharT >
		time_t string_2_time(const std::basic_string<CharT> &time, const CharT *format)
		{
			std::tm t = {0};
			std::basic_istringstream<CharT> ss(time);

			ss >> std::get_time(&t, format);
			
			return std::mktime(&t);
		}

		template < typename RepT, typename PeriodT >
		inline time_t duration_2_time(const duration<RepT, PeriodT> &val)
		{
			return stdex::time::system_clock::to_time_t(stdex::time::time_point<stdex::time::system_clock, duration<RepT, PeriodT>>(val));
		}

		template < typename T > 
		inline T time_2_duration(time_t t)
		{
			auto val = stdex::time::system_clock::from_time_t(t).time_since_epoch();
			return stdex::time::duration_cast<T>(val);
		}

		inline bool is_in_time_range(time_t beg, time_t end, time_t t)
		{
			auto init_time = [](time_t val)->std::uint32_t
			{
				tm tm_val = {0};
				::localtime_s(&tm_val, &val);
				
				return tm_val.tm_hour * 3600 + tm_val.tm_min * 60 + tm_val.tm_sec;
			};

			std::uint32_t beg_tmp = init_time(beg);
			std::uint32_t end_tmp = init_time(end);
			std::uint32_t t_tmp = init_time(t);

			return t > beg && t < end;
		}
	}
}


#endif
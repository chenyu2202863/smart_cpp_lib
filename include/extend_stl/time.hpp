#ifndef __EXTEND_STL_TIME_HPP
#define __EXTEND_STL_TIME_HPP

#include <ctime>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ratio>


namespace stdex { namespace time {

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


		inline std::time_t now()
		{
			return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		}

		template < typename CharT >
		std::basic_string<CharT> time_2_string(std::time_t t, const CharT *format)
		{
			std::basic_ostringstream<CharT> time_buffer;
			tm tm_val = {0};
			::localtime_s(&tm_val, &t);

			time_buffer << std::put_time(&tm_val, format);

			return time_buffer.str();
		}

		template < typename CharT >
		std::basic_string<CharT> time_2_string(std::time_t t)
		{
			return time_2_string<CharT>(t, (const CharT *)time_format());
		}

		template < typename CharT >
		std::time_t string_2_time(const std::basic_string<CharT> &time)
		{
			return string_2_time(time, (const CharT *)time_format());
		}

		template < typename CharT >
		std::time_t string_2_time(const std::basic_string<CharT> &time, const CharT *format)
		{
			std::tm t = {0};
			std::basic_istringstream<CharT> ss(time);

			ss >> std::get_time(&t, format);
			
			if( t.tm_year == 0 || 
				t.tm_mon == 0 || 
				t.tm_mday == 0 )
			{
				tm tm_val = {0};
				std::time_t now_time = now();
				::localtime_s(&tm_val, &now_time);

				t.tm_year = tm_val.tm_year;
				t.tm_mon = tm_val.tm_mon;
				t.tm_yday = tm_val.tm_yday;
				t.tm_wday = tm_val.tm_wday;
				t.tm_mday = tm_val.tm_mday;
				t.tm_isdst = tm_val.tm_isdst;
			}
				

			std::time_t ret = std::mktime(&t);
			assert(ret != -1);
			return ret;
		}

		template < typename RepT, typename PeriodT >
		inline std::time_t duration_2_time(const std::chrono::duration<RepT, PeriodT> &val)
		{
			return stdex::time::system_clock::to_time_t(stdex::time::time_point<stdex::time::system_clock, duration<RepT, PeriodT>>(val));
		}

		template < typename T > 
		inline T time_2_duration(time_t t)
		{
			auto val = stdex::time::system_clock::from_time_t(t).time_since_epoch();
			return stdex::time::duration_cast<T>(val);
		}

		inline bool is_in_time_range(std::time_t beg, std::time_t end, std::time_t t)
		{
			auto init_time = [](std::time_t val)->std::uint32_t
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

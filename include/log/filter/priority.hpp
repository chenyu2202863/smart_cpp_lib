#ifndef __LOG_PRIORITY_HPP
#define __LOG_PRIORITY_HPP


#include <type_traits>

namespace log_system
{
	namespace filter
	{

		// 日志级别

		enum
		{
			Critical = 0x00,
			Error,
			Warning,
			Notice,
			Info,
			Debug,
			Trace,
			All
		};

		

		// -----------------------------------------------------------
		// class PriorityFilter

		template < size_t _Priority >
		struct prio_filter_t
		{
			enum { BUFFER_RAW = 8, BUFFER_LEN = 16 };


			static bool is_print(size_t priority)
			{
				return priority < _Priority;
			}

			template < typename CharT >
			static const CharT *level(size_t priority)
			{
				assert(priority < All);

				const CharT (&text)[BUFFER_RAW][BUFFER_LEN] = _prio_text(CharT());

				return text[priority];
			}

		private:
			template<typename CharT>
			static const CharT *_prio_text();

			static const char (&_prio_text(char))[BUFFER_RAW][BUFFER_LEN]
			{
				static const char priorityText[BUFFER_RAW][BUFFER_LEN] = 
				{
					"(Critical)",
					"(Error)   ", 
					"(Warning) ",
					"(Notice)  ",
					"(Info)    ", 
					"(Debug)   ", 
					"(Trace)   ",
					"(All)     "
				};

				return priorityText;
			}

			
			static const wchar_t (&_prio_text(wchar_t))[BUFFER_RAW][BUFFER_LEN]
			{
				static const wchar_t priorityText[BUFFER_RAW][BUFFER_LEN] = 
				{
					L"(Critical)",
					L"(Error)   ", 
					L"(Warning) ",
					L"(Notice)  ",
					L"(Info)    ", 
					L"(Debug)   ", 
					L"(Trace)   ",
					L"(All)     "
				};

				return priorityText;
			}

		};

		typedef prio_filter_t<All>			debug_filter;
		typedef prio_filter_t<Debug>		release_filter;

#ifdef _DEBUG
		typedef debug_filter				default_filter;
#else			
		typedef release_filter				default_filter;
#endif

		
	}
}


#endif
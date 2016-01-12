#ifndef __LOG_BASIC_LOG_HPP
#define __LOG_BASIC_LOG_HPP

#include "../filter/priority.hpp"


namespace log_system
{
	namespace detail
	{
		template < typename CharT, typename T, typename U >
		struct selector_t;

		template < typename T, typename U >
		struct selector_t<char, T, U>
		{
			typedef T type;
			type &t_;

			selector_t(T &&t, U &&)
				: t_(t)
			{}
		};

		template < typename T, typename U >
		struct selector_t<wchar_t, T, U>
		{
			typedef U type;
			type &t_;

			selector_t(T &&, U &&u)
				: t_(u)
			{}
		};

		template < typename CharT, typename T, typename U >
		inline typename selector_t<CharT, T, U>::type &select(T &&t, U &&u)
		{
			return selector_t<CharT, T, U>(t, u).t_;
		}



		// --------------------------------------------------------
		// class basic_log
		// 提供格式化控制

		class basic_log
		{
		protected:
			size_t prio_level_;					// Priority Level
			bool show_level_;					// 是否一行结束

		public:
			basic_log()
				: prio_level_(filter::Info)
				, show_level_(true)
			{}

		public:
			void priority(size_t priority, bool show)
			{
				prio_level_ = priority;
				show_level_	= show;
			}

			size_t priority() const
			{
				return prio_level_;
			}

			bool is_show()
			{
				bool tmp = show_level_;
				show_level_ = false;

				return tmp;
			}
		};
	}
}








#endif
#ifndef __LOG_LOG_HPP
#define __LOG_LOG_HPP



#include <cstdlib>
#include <cstring>
#include <cstdarg>

#include "basic_log.hpp"
#include "../format/format.hpp"


namespace log_system
{
	
	namespace detail
	{
		// ---------------------------------------------------------------------
		// class Log

		template < typename StorageT, typename FilterT >
		class log_t
			: public basic_log
		{
		public:
			typedef StorageT	storage_type;
			typedef FilterT		filter_type;

			typedef typename storage_type::size_type	size_type;
			typedef typename storage_type::char_type	char_type;


		protected:
			storage_type storage_;			// StorageT
			
		private:
			enum { BUFFER_SIZE = 64 };

		public:
			log_t()
			{}

			template < typename ArgT >
			log_t(ArgT &&arg)
				: storage_(arg)
			{}

			template < typename ArgT1, typename ArgT2 >
			log_t(ArgT1 &&arg1, ArgT2 &&arg2)
				: storage_(arg1, arg2)
			{}

		public:
			storage_type &get_storage()
			{
				return storage_;
			}
			const storage_type &get_storage() const
			{
				return storage_;
			}

			bool is_good() const
			{
				return storage_.Good();
			}

			template < typename ArgT >
			void open(ArgT &&arg)
			{
				assert(!storage_.is_good());
				storage_.open(arg);
			}

			void close()
			{
				storage_.close();
			}

			void flush()
			{
				storage_.flush();
			}

			log_t &whitespace()
			{
				_print_string(' ');
				return *this;
			}

			log_t &new_line()
			{			
				_print_string('\n');

				return *this;
			}
			log_t &new_line(size_type count)
			{
				_print_string(count, '\n');

				return *this;
			}

						
			// Support endl,ends
			log_t &operator<<(log_t& (__cdecl *pFun)(log_t&))
			{
				return pFun(*this);
			}

			template < typename T >
			log_t &operator<<(const T &v)
			{
				return print(v);
			}

			template < typename ValueT >
			log_t &operator<<(const format::function_obj_t<ValueT> &obj)
			{
				obj(*this);

				return *this;
			}

		public:
			log_t &trace(const char_type *fmt, va_list arg)
			{			
				_print_string(fmt, arg);
				return *this;
			}
			log_t &trace(const char_type *fmt, ...)
			{			
				va_list args;
				va_start(args, fmt);

				_print_string(fmt, args);

				va_end(args);

				return *this;
			}

			// Print
			template < typename IterT >
			log_t &print(IterT first, IterT last)
			{			
				_print_string(first, last);

				return *this;
			}
			log_t &print(const std::basic_string<char_type> &str)
			{			
				_print_string(str);

				return *this;
			}
			template < typename T >
			log_t &print(const T &data, const char_type *fmt)
			{
				trace(fmt, data);

				return *this;
			}
			template < typename T >
			log_t &print(const T &data)
			{
				_print_string(data);

				return *this;
			}
			log_t &print(const char_type *pStr)
			{
				_print_string(pStr);

				return *this;
			}

		private:
			template < typename CharT >
			void _print_level(const CharT *text)
			{
				storage_.put(text, select<CharT>(std::strlen, std::wcslen)(text));
			}

			log_t &_print_string(char data)
			{
				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(data);
				}

				return *this;
			}
			log_t &_print_string(wchar_t data)
			{
				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(data);
				}

				return *this;
			}
			log_t &_print_string(int data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(itoa, _itow)(data, buf, 10);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(unsigned int data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(itoa, _itow)(data, buf, 10);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(short data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(itoa, _itow)(data, buf, 10);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(unsigned short data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(std::itoa, std::_itow)(buf, data);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));

					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(long data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(::itoa, ::_itow)(data, buf, 10);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(unsigned long data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(itoa, _itow)(buf, data);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(double data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(_snprintf_s, _snwprintf_s)(buf, BUFFER_SIZE, 10, "%f", data);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(long double data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(_snprintf_s, _snwprintf_s)(buf, BUFFER_SIZE, 10, "%lf", data);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}
			log_t &_print_string(__int64 data)
			{
				char_type buf[BUFFER_SIZE] = {0};
				select<char_type>(_snprintf_s, _snwprintf_s)(buf, BUFFER_SIZE, 10, 
					select<char_type>("%I64d", L"%I64d"), data);

				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(buf, select<char_type>(std::strlen, std::wcslen)(buf));
				}

				return *this;
			}

			template<typename IterT>
			log_t &_print_string(IterT first, IterT last)
			{			
				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(first, last - first);
				}

				return *this;
			}
			log_t &_print_string(const char_type *pStr)
			{
				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(pStr, select<char_type>(std::strlen, std::wcslen)(pStr));
				}
				return *this;
			}
			log_t &_print_string(const std::basic_string<char_type> &str)
			{			
				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));
					storage_.put(str.c_str(), str.size());
				}

				return *this;
			}
			log_t &_print_string(const char_type *szText, va_list arg)
			{
				if( filter_type::is_print(prio_level_) )
				{
					if( is_show() )
						_print_level(filter_type::level<char_type>(prio_level_));	
					storage_.put(szText, arg);
				}

				return *this;
			}
		};



		template < typename CharT, typename FilterT = filter::debug_filter, class StorageT = std::deque<storage::i_storage_t<CharT>*> >
		class multi_log_t
			: public detail::log_t<storage::multi_storage_t<CharT, StorageT>, FilterT >
		{
		public:
			template < typename T >
			void add(T &val)
			{
				static_assert(std::is_base_of<storage::i_storage_t<CharT>, typename T>::value, "T must be a i_storage_t");
				get_storage().add(val);
			}
		};
	}
	
}



#endif
#ifndef __LOG_STORAGE_HPP
#define __LOG_STORAGE_HPP


#include "storage_base.hpp"

#include <cassert>
#include <array>
#include <functional>


namespace log_system
{
	
	namespace storage
	{
		// -------------------------------------------------------------------------
		// class FileStorage

		template < typename CharT, bool _Own = true, template<typename> class BaseT = storage_base_t >
		class file_storage_t
			: public BaseT<CharT>
		{
		public:
			typedef BaseT<CharT>							base_storage_type;

			typedef typename base_storage_type::char_type	char_type;
			typedef typename base_storage_type::size_type	size_type;

		protected:
			FILE *file_;

		public:
			file_storage_t()
				: file_(NULL)
			{}
			file_storage_t(FILE *fp)
				: file_(fp)
			{}
			file_storage_t(const char_type * const szFile, bool bCheckOwn = true)
			{
				_open_impl(szFile, bCheckOwn);
			}
			template < typename LogT >
			file_storage_t(LogT &log)
				: file_(log.get_storage().file_)
			{}
			~file_storage_t()
			{
				if( _Own )
					close();
			}

		public:
			bool good() const
			{
				return file_ != NULL;
			}

			void open(const char_type * const file, bool bCheckOwn = true)
			{
				assert(file_	== NULL);
				_open_impl(file, bCheckOwn);
			}

			void close()
			{
				if( file_ != NULL )
				{
					::fclose(file_);
					file_ = NULL;
				}
			}

			void set_file(FILE *fp)
			{
				file_ = fp;
			}

		public:
			void put(char_type ch)
			{
				_put_impl(ch);
			}

			void put(size_type szCount, char_type ch)
			{
				_put_impl(szCount, ch);
			}

			void put(const char_type *pStr, size_type szCount)
			{
				_put_impl(pStr, szCount);
			}

			void put(const char_type *fmt, va_list args)
			{
				_put_impl(fmt, args);
			}

			void flush()
			{
				if( file_ != NULL )
					::fflush(file_);
			}

		private:
			void _open_impl(const char * const file, bool bCheckOwn)
			{
				assert(_Own || !bCheckOwn);

				file_ = ::_fsopen(file, "a+", SH_DENYNO);
			}
			void _open_impl(const wchar_t * const file, bool bCheckOwn)
			{
				assert(_Own || !bCheckOwn);

				file_ = ::_wfsopen(file, L"a+", SH_DENYNO);
			}

			void _put_impl(char ch)
			{
				::putc(ch, file_);
			}
			void _put_impl(wchar_t ch) 
			{
				::putwc(ch, file_);
			}

			void _put_impl(size_type szCount, char ch)
			{
				while(szCount--)
					::putc(ch, file_);
			}
			void _put_impl(size_type szCount, wchar_t ch)
			{
				while(szCount--)
					::putwc(ch, file_);
			}

			void _put_impl(const char *pStr, size_type szCount)
			{
				::fwrite(pStr, sizeof(char), szCount, file_);
			}
			void _put_impl(const wchar_t *pStr, size_type szCount) 
			{
				::fwrite(pStr, sizeof(wchar_t), szCount, file_);
			}

			void _put_impl(const char *fmt, va_list args)
			{
				::vfprintf(file_, fmt, args);
			}
			void _put_impl(const wchar_t *fmt, va_list args)
			{
				::vfwprintf(file_, fmt, args);
			}
		};	


		// -------------------------------------------------------------------------
		// class DebugStorageT

		template < typename CharT, template<typename> class BaseT = storage_base_t >
		class debug_storage_t
			: public BaseT<CharT>
		{
		public:
			typedef BaseT<CharT>					base_type;

			typedef typename base_type::char_type	char_type;
			typedef typename base_type::size_type	size_type;

		private:
			enum { LOG_BUFFER_SIZE = 512 };

		public:
			bool good() const
			{
				return true;
			}

		public:
			void put(char_type ch)
			{
				return _put_impl(ch);
			}

			void put(size_type szCount, char_type ch)
			{
				return _put_impl(szCount, ch);
			}

			void put(const char_type *pStr, size_type szCount)
			{
				return _put_impl(pStr, szCount);
			}

			void put(const char_type *fmt, va_list args)
			{
				_put_impl(fmt, args);
			}

			void flush()
			{
			}

		private:
			void _put_impl(char ch)
			{
				char buf[LOG_BUFFER_SIZE] = {0};
				sprintf_s(buf, "%c", ch);

				::OutputDebugStringA(buf);
			}
			void _put_impl(wchar_t ch) 
			{
				wchar_t buf[LOG_BUFFER_SIZE] = {0};
				swprintf_s(buf, L"%C", ch);

				::OutputDebugStringW(buf);
			}

			void _put_impl(size_type szCount, char ch)
			{
				char buf[LOG_BUFFER_SIZE] = {0};

				while(szCount--)
				{
					sprintf_s(buf, "%c", ch);
					::OutputDebugStringA(buf);
				}
			}
			void _put_impl(size_type szCount, wchar_t ch)
			{
				wchar_t buf[LOG_BUFFER_SIZE] = {0};

				while(szCount--)
				{
					swprintf_s(buf, L"%C", ch);
					::OutputDebugStringW(buf);
				}
			}

			void _put_impl(const char *pStr, size_type szCount)
			{
				::OutputDebugStringA(pStr);
			}

			void _put_impl(const wchar_t *pStr, size_type szCount) 
			{
				::OutputDebugStringW(pStr);
			}

			void _put_impl(const char *fmt, va_list args)
			{
				char buf[LOG_BUFFER_SIZE] = {0};
				_vsnprintf_s(buf, LOG_BUFFER_SIZE, fmt, args);

				::OutputDebugStringA(buf);
			}

			void _put_impl(const wchar_t *fmt, va_list args)
			{
				wchar_t buf[LOG_BUFFER_SIZE] = {0};
				_vsnwprintf_s(buf, LOG_BUFFER_SIZE, fmt, args);

				::OutputDebugStringW(buf);
			}
		};

		

		// console storage

		template < typename CharT, template<typename> class BaseT = storage_base_t >
		class console_storage_t
			: public BaseT<CharT>
		{
		public:
			typedef BaseT<CharT>					base_type;

			typedef typename base_type::char_type	char_type;
			typedef typename base_type::size_type	size_type;


		public:
			bool good() const
			{
				return std::cout.good();
			}

		public:
			void put(char_type ch)
			{
				detail::select<char_type>(std::cout, std::wcout) << ch;
			}

			void put(size_type szCount, char_type ch)
			{
				while(--szCount)
					detail::select<char_type>(std::cout, std::wcout) << ch;
			}

			void put(const char_type *pStr, size_type szCount)
			{
				detail::select<char_type>(std::cout, std::wcout).write(pStr, szCount);
			}

			void put(const char_type *fmt, va_list args)
			{
				char_type buf[LOG_BUFFER_SIZE] = {0};
				size_t len = detail::select<char_type>(_vsnprintf_s, _vsnwprintf_s)(buf, LOG_BUFFER_SIZE, fmt, args);
			
				detail::select<char_type>(std::cout, std::wcout).write(buf, len);
			}

			void flush()
			{
				detail::select<char_type>(std::cout, std::wcout) << std::flush;
			}
		};



		// -------------------------------------------------------------------------
		// class StringStorage

		template < typename CharT, template<typename> class BaseT = storage_base_t >
		class string_storage_t
			: public BaseT<CharT>
		{
		public:
			typedef BaseT<CharT>					base_type;

			typedef typename base_type::char_type	char_type;
			typedef typename base_type::size_type	size_type;

			typedef std::basic_string<char_type>	storage_type;

		private:
			enum { LOG_BUFFER_SIZE = 1024 };
			storage_type storage_;

		public:
			operator storage_type &()
			{
				return storage_;
			}
			operator const storage_type &() const
			{
				return storage_;
			}

		public:
			void put(char_type ch)
			{
				storage_.append(1, ch);
			}

			void put(size_type count, char_type ch)
			{
				storage_.append(count, ch);
			}

			void put(const char_type *pStr, size_type count)
			{
				storage_.append(pStr, pStr + count);
			}

			void put(const char_type *fmt, va_list args)
			{
				return _put_impl(fmt, args);
			}

			void flush()
			{

			}

		private:
			void _put_impl(const char *fmt, va_list args)
			{
				char buf[LOG_BUFFER_SIZE] = {0};
				int cch = _vsnprintf_s(buf, LOG_BUFFER_SIZE, fmt, args);
				if( cch	< 0 )
					cch = LOG_BUFFER_SIZE;

				storage_.append(buf, cch);
			}
			void _put_impl(const wchar_t *fmt, va_list args)
			{
				wchar_t buf[LOG_BUFFER_SIZE] = {0};
				int cch = _vsnwprintf_s(buf, LOG_BUFFER_SIZE, fmt, args);
				if( cch	< 0 )
					cch = LOG_BUFFER_SIZE;

				storage_.append(buf, cch);
			}
		};



		// -------------------------------------------------------------------------
		// class MultiStorage

		template < typename CharT, typename StorageT = std::deque<i_storage_t<CharT> *> >
		class multi_storage_t
			: public storage_base_t<CharT>
		{
			typedef multi_storage_t<CharT, StorageT>		this_type;
			typedef typename StorageT::value_type			value_type;

		public:
			typedef typename storage_base_t<CharT>::char_type		char_type;
			typedef typename storage_base_t<CharT>::size_type		size_type;

		private:
			StorageT storages_;
			size_t pos_;

		public:
			multi_storage_t()
				: pos_(0)
			{}

		public:
			void clear()
			{
				storages_.clear();
				pos_ = 0;
			}

			void add(i_storage_t<char_type> &stg)
			{
				storages_.push_back(&stg);
			}

		public:
			void put(char_type ch)
			{
				std::for_each(storages_.begin(), storages_.end(), [ch](const StorageT::value_type &v)
				{
					v->put(ch);
				});
			}

			void put(size_type count, char_type ch)
			{
				std::for_each(storages_.begin(), storages_.end(), [count, ch](const StorageT::value_type &v)
				{
					v->put(count, ch);
				});
			}

			void put(const char_type *pStr, size_type count)
			{
				std::for_each(storages_.begin(), storages_.end(), [pStr, count](const StorageT::value_type &v)
				{
					v->put(pStr, count);
				});
			}

			void put(const char_type *fmt, va_list args)
			{
				std::for_each(storages_.begin(), storages_.end(), [fmt, &args](const StorageT::value_type &v)
				{
					v->put(fmt, args);
				});
			}

			void flush()
			{
				std::for_each(storages_.begin(), storages_.end(),
					std::tr1::bind(&i_storage_t<CharT>::flush, std::tr1::placeholders::_1));
			}
		};
	}


	
}




#endif 
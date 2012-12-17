#ifndef __LOG_CACHE_LOG_HPP
#define __LOG_CACHE_LOG_HPP

#include "storage_base.hpp"
#include <array>


#pragma warning (disable : 4996)

namespace log_system
{

	namespace storage
	{
		// ---------------------------------------------------------------------
		// class CacheStorage

		template < size_t _CacheSize, typename ImplStorageT, typename BaseT = storage_base_t<typename ImplStorageT::char_type> >
		class cache_storage_t
			: public BaseT
		{
			typedef BaseT									storage_type;
			typedef ImplStorageT							impl_type;

		public:
			typedef typename storage_type::char_type		char_type;
			typedef typename storage_type::size_type		size_type;

		private:
			impl_type impl_;
			std::tr1::array<char_type, _CacheSize> cache_;
			size_t pos_;

			// 解析不定值参数时的缓冲区
			enum { BUFFER_SIZE = 512 };

		public:
			cache_storage_t()
				: pos_(0)
			{
				std::fill_n(cache_.begin(), _CacheSize, 0);
			}
			template < typename ArgT >
			cache_storage_t(ArgT &&arg)
				: impl_(arg)
				, pos_(0)
			{
				std::fill_n(cache_.begin(), _CacheSize, 0);
			}
			template < typename ArgT1, typename ArgT2 >
			cache_storage_t(ArgT1 &&arg1, ArgT2 &&arg2)
				: impl_(arg1, arg2)
				, pos_(0)
			{
				std::fill_n(cache_.begin(), _CacheSize, 0);
			}

		public:
			void put(char_type ch)
			{
				if( _is_full(1) )
					flush();

				_push(ch);
			}

			void put(size_t count, char_type ch)
			{
				if( _is_full(count) )
					flush();

				_push(count, ch);
			}

			void put(const char_type *pStr, size_t count)
			{
				if( _is_full(count) )
					flush();

				_push(pStr, count);
			}

			void put(const char_type *pStr, va_list args)
			{
				static_assert("no implement", false);
				// Warning... NonImplement
			}

			void flush()
			{
				impl_.put(cache_.data(), pos_);
				impl_.flush();
				pos_ = 0;
			}


		private:
			// 判断缓冲区是否够容纳size大小的数据
			bool _is_full(size_t size)
			{
				if( pos_ + size < _CacheSize )
					return false;
				else
					return true;
			}

			void _push(char_type c)
			{
				cache_[pos_++] = c;
			}
			void _push(size_t count, char_type ch)
			{
				for(; count >= 0; --count)
				{
					if( pos_ >= _CacheSize )
						flush();

					cache_[pos_++] = ch;
				}
			}
			void _push(const char_type *pStr, size_t count)
			{
				// 如果缓冲区足够，则写入缓冲
				if( count + pos_ < _CacheSize )
				{
					std::copy(pStr, pStr + count, 
						stdext::checked_array_iterator<char_type *>(&cache_[pos_], _CacheSize - pos_));
					pos_ += count;
				}
				// 缓冲区不足
				else
				{		
					// 如果缓冲区不够容纳当前写入数据,则直接写入设备
					if( count > _CacheSize )
					{
						// 把缓冲的数据写入设备
						flush();
						impl_.put(pStr, count);
					}
					// 写入缓冲
					else
                    {
						std::copy(pStr, pStr + count, 
							stdext::make_checked_array_iterator(&cache_[pos_], _CacheSize - pos_));
						pos_ += count;
					}
				}
			}

			void _push(const char *fmt, va_list args)
			{
				char buf[BUFFER_SIZE] = {0};
				int cch = _vsnprintf_s(buf, BUFFER_SIZE, fmt, args);
				if( cch	< 0 )
				{
					assert(0);
					cch = 0;
				}

				std::copy(buf, buf + std::strlen(buf), &cache_[pos_]);
				pos_ += cch;
			}
			void _push(const wchar_t *fmt, va_list args)
			{
				wchar_t buf[BUFFER_SIZE] = {0};
				int cch = _vsnwprintf_s(buf, BUFFER_SIZE, fmt, args);
				if( cch	< 0 )
				{
					assert(0);
					cch = 0;
				}

				std::copy(buf, buf + std::wcslen(buf), &cache_[pos_]);
				pos_ += cch;
			}
		};
	}
	
}


#endif
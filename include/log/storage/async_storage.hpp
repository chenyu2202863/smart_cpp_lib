#ifndef __LOG_ASYNC_STORAGE_LOG_HPP
#define __LOG_ASYNC_STORAGE_LOG_HPP

#include "storage_base.hpp"
#include "../../extend_stl/container/blocking_queue.hpp"
#include "../../multi_thread/thread.hpp"
#include "../../multi_thread/lock.hpp"


namespace log_system
{

	namespace storage
	{
		// ---------------------------------------------------------------------
		// class async_storage_t

		template < typename ImplStorageT, typename BaseT = storage_base_t<typename ImplStorageT::char_type> >
		class async_storage_t
			: public BaseT
		{
			typedef BaseT									storage_type;
			typedef ImplStorageT							impl_type;

		public:
			typedef typename storage_type::char_type		char_type;
			typedef typename storage_type::size_type		size_type;

			typedef stdex::container::blocking_queue_t<std::basic_string<char_type>> messages_type;

		private:
			impl_type impl_;

			messages_type msgs_;

			multi_thread::thread_impl_ex thread_;
			multi_thread::event_t exit_;

		public:
			async_storage_t()
			{
				_start();
			}
			template < typename ArgT >
			async_storage_t(ArgT &&arg)
				: impl_(arg)
			{
				_start();
			}
			template < typename ArgT1, typename ArgT2 >
			async_storage_t(ArgT1 &&arg1, ArgT2 &&arg2)
				: impl_(arg1, arg2)
			{
				_start();
			}

			~async_storage_t()
			{
				_stop();
			}

		public:
			void put(char_type ch)
			{
				msgs_.put(std::basic_string<char_type>(1, ch));
			}

			void put(size_t count, char_type ch)
			{
				msgs_.put(std::basic_string<char_type>(count, ch));
			}

			void put(const char_type *pStr, size_t count)
			{
				msgs_.put(std::basic_string<char_type>(pStr, count));
			}

			void put(const char_type *pStr, va_list args)
			{
				char_type buf[1024] = {0};
				size_t len = detail::select<char_type>(_vsnprintf_s, _vsnwprintf_s)(buf, LOG_BUFFER_SIZE, fmt, args);
			
				msgs_.put(std::basic_string<char_type>(buf, len));
			}

			void flush()
			{
				impl_.flush();
			}

		private:
			void _start()
			{
				exit_.create(0, TRUE);

				thread_.register_callback(std::bind(&async_storage_t::_thread_impl, this));
				thread_.start();
			}

			void _stop()
			{
				exit_.set_event();
				msgs_.put("");
				thread_.stop();
			}

			DWORD _thread_impl()
			{
				while(1)
				{
					auto val = msgs_.get();
					if( msgs_.empty() && exit_.is_signalled() )
						break;

					impl_.put(val.c_str(), val.length());
				}

				return 0;
			}
		};
	}

}


#endif
#ifndef __LOG_THREAD_SAFE_HPP
#define __LOG_THREAD_SAFE_HPP

#include "../../multi_thread/lock.hpp"

namespace log_system
{

	namespace detail
	{
		// -----------------------------------------------------
		// class ThreadSafeLog

		template < typename LogT >
		class thread_safe_agent_t
		{
			typedef multi_thread::spin_lock lock_type;

			lock_type lock_;
			LogT &log_;

		public:
			typedef typename LogT::char_type char_type;
			typedef typename LogT::size_type size_type;

		public:
			thread_safe_agent_t(LogT &log)
				: log_(log)
			{
				lock_.lock();
			}
			~thread_safe_agent_t()
			{
				lock_.unlock();
			}

		public:
			LogT &get()
			{
				return log_;
			}

			const LogT &get() const
			{
				return log_;
			}

			thread_safe_agent_t &operator<<(thread_safe_agent_t& (__cdecl *pFun)(thread_safe_agent_t&))
			{
				return pFun(*this);
			}

		public:
			void flush()
			{
				log_.flush();
			}

			thread_safe_agent_t &whitespace()
			{
				log_.whitespace();
				return *this;
			}

			thread_safe_agent_t &new_line()
			{			
				log_.new_line();

				return *this;
			}
			thread_safe_agent_t &new_line(size_type count)
			{
				log_.new_line(count);

				return *this;
			}
		};

		template < typename LogT >
		inline thread_safe_agent_t<LogT> safe(LogT &log)
		{
			return thread_safe_agent_t<LogT>(log);
		}

		template < typename LogT, typename T >
		inline thread_safe_agent_t<LogT> &operator<<(thread_safe_agent_t<LogT> &log, const T &val)
		{
			log.get() << val;
			return log;
		}

	}
	
}


#endif
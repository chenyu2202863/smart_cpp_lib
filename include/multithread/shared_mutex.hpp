#ifndef __MULTI_THREAD_SHARED_MUTEX_HPP
#define __MULTI_THREAD_SHARED_MUTEX_HPP


#include <memory>

namespace multi_thread {

	class shared_mutex_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		shared_mutex_t();
		~shared_mutex_t();

		shared_mutex_t(const shared_mutex_t &) = delete;
		shared_mutex_t &operator=(const shared_mutex_t &) = delete;

		void lock();
		void unlock();

		void lock_shared();
		void unlock_shared();
	};

	
	struct shared_lock_t
	{
		shared_mutex_t &mutex_;

		shared_lock_t(shared_mutex_t &mutex)
			: mutex_(mutex)
		{
			mutex_.lock_shared();
		}

		~shared_lock_t()
		{
			mutex_.unlock_shared();
		}
	};

	struct exclusive_lock_t
	{
		shared_mutex_t &mutex_;

		exclusive_lock_t(shared_mutex_t &mutex)
			: mutex_(mutex)
		{
			mutex_.lock();
		}

		~exclusive_lock_t()
		{
			mutex_.unlock();
		}
	};
}

#endif
#include "shared_mutex.hpp"

#include <Windows.h>

namespace multi_thread {

	struct shared_mutex_t::impl
	{
		SRWLOCK rw_lock_;

		impl()
		{
			::InitializeSRWLock(&rw_lock_);
		}
	};


	shared_mutex_t::shared_mutex_t()
		: impl_(std::make_unique<impl>())
	{

	}
	shared_mutex_t::~shared_mutex_t()
	{}

		
	void shared_mutex_t::lock()
	{
		::AcquireSRWLockExclusive(&impl_->rw_lock_);
	}

	void shared_mutex_t::unlock()
	{
		::ReleaseSRWLockExclusive(&impl_->rw_lock_);
	}

	void shared_mutex_t::lock_shared()
	{
		::AcquireSRWLockShared(&impl_->rw_lock_);
	}

	void shared_mutex_t::unlock_shared()
	{
		::ReleaseSRWLockShared(&impl_->rw_lock_);
	}


}
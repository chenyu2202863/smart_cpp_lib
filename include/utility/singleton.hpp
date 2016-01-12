#ifndef __UTILITY_SINGLETON_HPP
#define __UTILITY_SINGLETON_HPP

#include <mutex>

namespace utility {

	template < typename T >
	struct singleton_t
	{
		static std::unique_ptr<T> instance_;
		static std::once_flag flag_;

		static T &instance()
		{
			std::call_once(flag_, []()
			{
				if( !instance_ )
					instance_ = std::make_unique<T>();
			});

			return *instance_;
		}
	};

	template < typename T >
	std::once_flag singleton_t<T>::flag_;

	template < typename T >
	std::unique_ptr<T> singleton_t<T>::instance_;
}

#endif


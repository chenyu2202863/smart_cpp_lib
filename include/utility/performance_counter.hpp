#ifndef __UTILITY_PERFORMANCE_COUNTER_HPP
#define __UTILITY_PERFORMANCE_COUNTER_HPP

#include <chrono>

namespace utility {

	struct performance_t
	{
		std::chrono::system_clock::time_point start_;

		performance_t()
		{
			start_ = std::chrono::high_resolution_clock::now();
		}

		std::chrono::milliseconds time() const
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_);
		}
	};
}

#endif
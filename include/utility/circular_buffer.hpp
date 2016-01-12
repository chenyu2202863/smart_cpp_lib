#ifndef __UTILITY_CICULAR_BUFFER_HPP
#define __UTILITY_CICULAR_BUFFER_HPP

#include <array>
#include <cstdint>
#include <atomic>
#include <memory>
#include <mutex>

namespace utility {

	template < typename T, std::uint32_t N >
	class cicular_buffer_t
	{
		typedef std::shared_ptr<T> value_t;

		mutable std::mutex mutex_;
		std::array<value_t, N> buffer_;
		std::atomic<std::uint32_t> index_;

	public:
		template < typename AllocatorT = std::allocator<T> >
		cicular_buffer_t(const AllocatorT &allocator = AllocatorT())
			: index_(1)
		{
			for(auto i = 0; i != N; ++i)
				buffer_[i] = std::allocate_shared<T>(allocator);
		}

	private:
		cicular_buffer_t(const cicular_buffer_t &);
		cicular_buffer_t &operator=(const cicular_buffer_t &);

	public:
		value_t back() const
		{
			std::lock_guard<std::mutex> lock(mutex_);

			return buffer_[index_];
		}


		void push_back(value_t && val)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			++index_;
			if( index_ >= N )
				index_ = 0;

			buffer_[index_] = std::move(val);
		}

		template < typename HandlerT >
		void for_each(HandlerT && handler)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			std::for_each(buffer_.cbegin(), buffer_.cend(), 
						  std::forward<HandlerT>(handler));
		}
	};
}

#endif
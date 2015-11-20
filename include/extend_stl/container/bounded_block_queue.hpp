#ifndef __CONTAINER_BOUNDED_BLOCKING_QUEUE_HPP
#define __CONTAINER_BOUNDED_BLOCKING_QUEUE_HPP

#include <mutex>
#include <queue>
#include <chrono>


/*
限制大小的阻塞队列，适用于生产者消费者

bounded_block_queue_t



*/

namespace stdex { namespace container {

		template< typename T, typename A = std::allocator<T> >
		class bounded_block_queue_t
		{
			typedef std::unique_lock<std::mutex> auto_lock;

			mutable std::mutex mutex_;
			std::condition_variable not_empty_;
			std::condition_variable not_full_;
			std::deque<T, A> queue_;

			std::uint32_t max_size_;

		public:
			explicit bounded_block_queue_t(std::uint32_t maxSize)
				: max_size_(maxSize)
			{} 
			bounded_block_queue_t(std::uint32_t maxSize, A &allocator)
				: max_size_(maxSize)
				, queue_(allocator)
			{}

		private:
			bounded_block_queue_t(const bounded_block_queue_t &);
			bounded_block_queue_t &operator=(const bounded_block_queue_t &);

		public:
			template < typename U, typename HandlerT >
			void put_impl(U &&x, HandlerT &&handler)
			{
				{
					auto_lock lock(mutex_);
					while( queue_.size() == max_size_ )
						handler(lock);

					assert(queue_.size() != max_size_);
					queue_.push_back(x);
				}

				not_empty_.notify_one();
			}

			template < typename U >
			void put(U &&x)
			{
				put_impl(std::forward<U>(x), [this](auto &lock)
				{
					not_full_.wait(lock);
				});
			}

			template < typename U >
			void put(U &&x, std::chrono::milliseconds time_out)
			{
				put_impl(std::forward<U>(x), [this, time_out](auto &lock)
				{
					not_full_.wait_for(lock, time_out);
				});
			}


			T get()
			{
				T front;

				{
					auto_lock lock(mutex_);
					while(queue_.empty())
						not_empty_.wait(lock);

					assert(!queue_.empty());
					front = queue_.front();
					queue_.pop_front();
				}

				not_full_.notify_one();
				return front;
			}

			bool empty() const
			{
				auto_lock lock(mutex_);
				return queue_.empty();
			}

			bool full() const
			{
				auto_lock lock(mutex_);
				return queue_.size() == max_size_;
			}

			size_t size() const
			{
				auto_lock lock(mutex_);
				return queue_.size();
			}
		};


	}
}

#endif  


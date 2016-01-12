#ifndef __CONTAINER_BOUNDED_QUEUE_HPP
#define __CONTAINER_BOUNDED_QUEUE_HPP


/** @blocking_queue.hpp
*
* @author <³ÂìÏ>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* Éú²úÕßÏû·ÑÕßÈÝÆ÷
*/

#include <mutex>
#include <queue>
#include <condition_variable>
#include <cassert>

/*
×èÈû¶ÓÁÐ£¬ÊÊÓÃÓÚÉú²úÕßÏû·ÑÕß

	block_queue_t

*/
namespace stdex
{
	namespace container
	{
		/**
		* @class <sync_sequence_container_t>
		* @brief Éú²úÕßÏû·ÑÕßÈÝÆ÷£¬½Ó¿ÚÓëstlÈÝÆ÷ÀàËÆ£¬²ÉÓÃFIFOËã·¨
		*
		* T ÖµÀàÐÍ
		* A ÄÚ´æ·ÖÅäÆ÷£¬ÔÚ¸ßÐÔÄÜµÄµØ·½ÐèÒª×Ô¼ºÌá¹©ÄÚ´æ·ÖÅäÆ÷
		*/

		template< typename T, typename A = std::allocator<T> >
		class blocking_queue_t
		{
			typedef std::mutex					Mutex;
			typedef std::unique_lock<Mutex>		AutoLock;
			typedef std::condition_variable		Condtion;
			typedef std::deque<T, A>			Container;

			mutable Mutex mutex_;
			Condtion not_empty_;
			Container queue_;

		public:
			blocking_queue_t()
			{} 

			/**
			* @brief ´«ÈëÒ»¸öallocator
			* @param <alloc> <allocator¶ÔÏó>
			* @exception <²»»áÅ×³öÈÎºÎÒì³£>
			* @return <ÎÞ>
			* @note <ÎÞ>
			* @remarks <Ìá¸ßÄÚ´æ·ÖÅäÐ§ÂÊ>
			*/
			explicit blocking_queue_t(A &allocator)
				: queue_(allocator)
			{}

			blocking_queue_t(const blocking_queue_t &) = delete;
			blocking_queue_t &operator=(const blocking_queue_t &) = delete;

		public:
			/**
			* @brief °ÑÊý¾ÝÑ¹Èë¶ÓÁÐ£¬Éú²úÒ»¸öÊý¾Ý
			* @param <x> <Ñ¹ÈëÊý¾Ý>
			* @exception <²»»áÅ×³öÈÎºÎÒì³£>
			* @return <ÎÞ>
			* @note <Ïß³Ì°²È«£¬¿É²¢·¢¶à´Îµ÷ÓÃ>
			* @remarks <ÎÞ>
			*/
			void put(T &&x)
			{
				{
					AutoLock lock(mutex_);
					queue_.emplace_back(std::move(x));
				}

				not_empty_.notify_one();
			}

			void put(const T &x)
			{
				{
					AutoLock lock(mutex_);
					queue_.emplace_back(x);
				}

				not_empty_.notify_one();
			}

			/**
			* @brief °ÑÊý¾Ýµ¯³ö¶ÓÁÐ£¬Ïû·ÑÒ»¸öÊý¾Ý
			* @param <ÎÞ>
			* @exception <²»»áÅ×³öÈÎºÎÒì³£>
			* @return <µ¯³öÒ»¸öÊý¾Ý>
			* @note <Ïß³Ì°²È«£¬¿É²¢·¢¶à´Îµ÷ÓÃ>
			* @remarks <ÎÞ>
			*/
			T get()
			{
				T front;
				{
					AutoLock lock(mutex_);
					while(queue_.empty())
					{
						not_empty_.wait(lock);
					}
					assert(!queue_.empty());
					front = std::move(queue_.front());
					queue_.pop_front();
				}

				return front;
			}

			size_t size() const
			{
				AutoLock lock(mutex_);
				return queue_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return queue_.empty();
			}

			/**
			* @brief ±éÀú¶ÓÁÐ
			* @param <func> <funcµ÷ÓÃÔ¼¶¨Îªvoid(const T &val)>
			* @exception <²»»áÅ×³öÈÎºÎÒì³£>
			* @return <ÎÞ>
			* @note <ÎÞ>
			* @remarks <ÎÞ>
			*/
			template < typename FuncT >
			void for_each(const FuncT &func)
			{
				AutoLock lock(mutex_);
				std::for_each(queue_.begin(), queue_.end(), func);
			}
		};

	}

}

#endif  


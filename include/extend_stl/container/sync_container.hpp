#ifndef __CONTAINER_SYNC_CONTAINER_HPP
#define __CONTAINER_SYNC_CONTAINER_HPP

/** @async_container.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* 线程安全线性容器,提供线性容器和关联容器
*/


#include <vector>
#include <map>
#include <mutex>
#include <algorithm>

/*
线程安全线性容器(vector, list, deque)
sync_sequence_container_t

线程安全关联容器(map, set, multimap, multiset, hash)
sync_assoc_container_t

*/


namespace stdex
{


	namespace container
	{
		/**
		* @class <sync_sequence_container_t>
		* @brief 线性容器，线程安全，接口与stl容器类似
		*
		* T 值类型
		* C 容器类型，可以vector、list、deque，默认使用vector
		* S 同步类型，可以critical_section、event_t、mutex
		*/

		template < 
			typename T,
			typename C = std::vector<T>, 
			typename S = std::mutex
		>
		class sync_sequence_container_t
		{
		public:
			typedef S Mutex;
			typedef std::unique_lock<Mutex>	AutoLock;
			typedef C Container;

			typedef typename Container::value_type		value_type;
			typedef typename Container::iterator		iterator;
			typedef typename Container::const_iterator	const_iterator;
			typedef typename Container::reference		reference;
			typedef typename Container::const_reference	const_reference;
			typedef typename Container::allocator_type	allocator_type;

		private:
			mutable Mutex mutex_;
			Container container_;

		public:
			sync_sequence_container_t()
			{}

			explicit sync_sequence_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

			sync_sequence_container_t(const sync_sequence_container_t &) = delete;
			sync_sequence_container_t &operator=(const sync_sequence_container_t &) = delete;

		public:
			size_t size() const
			{
				AutoLock lock(mutex_);
				return container_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return container_.empty();
			}

			void clear()
			{
				AutoLock lock(mutex_);
				container_.clear();
			}

			void pop_top()
			{
				AutoLock lock(mutex_);
				container_.pop_front();
			}

			void push_front(const T &val)
			{
				AutoLock lock(mutex_);
				container_.push_front(val);
			}
			void push_back(const T &val)
			{
				AutoLock lock(mutex_);
				container_.push_back(val);
			}

			template < typename ...U >
			void emplace_back(U &&...val)
			{
				AutoLock lock(mutex);
				container_.emplace_back(std::forward<U>(val)...);
			}

			template < typename OP >
			void for_each(const OP &op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}


			template < typename Functor, typename OP >
			bool op_if(const Functor &func, const OP &op)
			{
				AutoLock lock(mutex_);
				iterator iter = find_if(func);
				if( iter != container_.end() )
				{
					op(*iter);
					return true;
				}

				return false;
			}

	
			template < typename Functor, typename OP1, typename OP2 >
			void op_if(const Functor &func, const OP1 &op1, const OP2 &op2)
			{
				AutoLock lock(mutex_);
				iterator iter = std::find_if(container_.begin(), container_.end(), func);
				if( iter != container_.end() )
					op1(*iter);
				else
					op2();
			}

			template < typename OP >
			void erase(const OP &op)
			{
				AutoLock lock(mutex_);
				const_iterator iter = find_if(op);
				if( iter != container_.end() )
					container_.erase(iter);
			}

			void sort()
			{
				AutoLock lock(mutex_);
				std::sort(container_.begin(), container_.end());
			}

			template < typename OP >
			void sort(const OP &op)
			{
				AutoLock lock(mutex_);
				std::sort(container_.begin(), container_.end(), op);
			}

		};



		/**
		* @class <sync_assoc_container_t>
		* @brief 关联容器，线程安全，接口与stl容器类似
		*
		* K key类型
		* V value类型
		* C 容器类型，可以map、set、multi_map、multi_set、unordered_map、unordered_set
		* S 同步类型，可以critical_section、event_t、mutex
		*/

		template < 
			typename K, 
			typename V,
			typename C = std::map<K, V>, 
			typename S = std::mutex
		>
		class sync_assoc_container_t
		{
		public:
			typedef S Mutex;
			typedef std::unique_lock<Mutex>	AutoLock;
			typedef C Container;

			typedef typename Container::key_type		key_type;
			typedef typename Container::mapped_type		mapped_type;
			typedef typename Container::value_type		value_type;
			typedef typename Container::iterator		iterator;
			typedef typename Container::const_iterator	const_iterator;
			typedef typename Container::reference		reference;
			typedef typename Container::const_reference	const_reference;
			typedef typename Container::allocator_type	allocator_type;

		private:
			mutable Mutex mutex_;
			Container container_;

		public:
			sync_assoc_container_t()
			{}

			/**
			* @brief 传入一个allocator
			* @param <alloc> <allocator对象>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <根据传入容器类型，定制容器内存分配器>
			* @remarks <提高内存分配效率>
			*/
			explicit sync_assoc_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

			sync_assoc_container_t(const sync_assoc_container_t &) = delete;
			sync_assoc_container_t &operator=(const sync_assoc_container_t &) = delete;

		public:
			size_t size() const
			{
				AutoLock lock(mutex_);
				return container_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return container_.empty();
			}

			void clear()
			{
				AutoLock lock(mutex_);
				container_.clear();
			}

			mapped_type &operator[](const key_type &key)
			{
				AutoLock lock(mutex_);
				return container_[key];
			}

			const mapped_type &operator[](const key_type &key) const
			{
				AutoLock lock(mutex_);
				return container_.at(key);
			}

			bool exsit(const key_type &key) const
			{
				AutoLock lock(mutex_);
				return container_.find(key) != container_.end();
			}

			void insert(key_type &&key, mapped_type &&val)
			{
				AutoLock lock(mutex_);
				container_.emplace(std::move(key), std::move(val));
			}

			void insert(const key_type &key, const mapped_type &val)
			{
				AutoLock lock(mutex_);
				container_.insert(std::make_pair(key, val));
			}

			template < typename OP >
			void for_each(const OP &op) const
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			void for_each(OP op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

		
			/**
			* @brief 如果在容器中找不到key，则执行op
			* @param <key> <key关键字>
			* @param <op> <回调函数参数，接受一个const value_type &参数>
			* @exception <不会抛出任何异常>
			* @return <如果执行op，则返回true，否则返回false>
			* @note <无>
			* @remarks <无>
			*/
			template < typename OP >
			bool not_if_op(const key_type &key, OP &&op)
			{
				AutoLock lock(mutex_);
				const_iterator iter = container_.find(key);
				if( iter == container_.end() )
				{
					op();
					return true;
				}

				return false;
			}

			/**
			* @brief 如果在容器中找到key，则执行op
			* @param <key> <key关键字>
			* @param <op> <回调函数参数，接受一个const value_type &参数>
			* @exception <不会抛出任何异常>
			* @return <如果执行op，则返回true，否则返回false>
			* @note <无>
			* @remarks <无>
			*/
			template < typename OP >
			bool op_if(const key_type &key, OP op)
			{
				AutoLock lock(mutex_);
				iterator iter = container_.find(key);
				if( iter != container_.end() )
				{
					op(*iter);
					return true;
				}

				return false;
			}

			void erase(const key_type &key)
			{
				AutoLock lock(mutex_);

				container_.erase(key);
			}
		};

	}

}



#endif
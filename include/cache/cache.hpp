#ifndef __CACHE_HPP
#define __CACHE_HPP 

#include <cassert> 

#include "../multi_thread/lock.hpp"
#include "detail/lfu_impl.hpp"
#include "detail/lru_impl.hpp"
#include "detail/fifo_impl.hpp"


#pragma warning(disable: 4503)

namespace cache
{

	// 缓存策略
	/*
	1. 按过期时间
	2. 按间隔时间
	3. 依赖项
	4. LRU/MRU

	*/

	template< 
		typename K, 
		typename V,
		template< typename, typename > class CacheImplT, 
		typename L = multi_thread::critical_section
	>
	class cache_t
	{ 
	public: 
		typedef K key_type; 
		typedef V value_type; 
		typedef void (*deletor_type)(const key_type &, const value_type &);
		typedef L mutex_type;

		typedef CacheImplT<key_type, std::pair<value_type, deletor_type>> impl_type;

		typedef typename impl_type::map_iterator			map_iterator;
		typedef typename impl_type::map_const_iterator		map_const_iterator;

		typedef multi_thread::auto_lock_t<mutex_type>		auto_lock;

	private:
		size_t capacity_;			
		size_t size_;

		impl_type impl_;
		mutable mutex_type mutex_;

	public:
		explicit cache_t(size_t c)
			: impl_(c)
			, capacity_(c)
			, size_(0)
		{

		}

		~cache_t()
		{
			clear();
		}

	private:
		cache_t(const cache_t &);
		cache_t &operator=(const cache_t &);

	public:
		map_iterator begin()
		{
			auto_lock lock(mutex_);
			return impl_.begin();
		}

		map_const_iterator begin() const
		{
			auto_lock lock(mutex_);
			return impl_.begin();
		}

		map_iterator end()
		{
			auto_lock lock(mutex_);
			return impl_.end();
		}

		map_const_iterator end() const
		{
			auto_lock lock(mutex_);
			return impl_.end();
		}

	public:
		void insert(const key_type &k, const value_type &v, deletor_type deletor = 0)
		{
			auto_lock lock(mutex_);

			if( capacity_ == size_ )
			{
				impl_.evict();
				--size_;
			}

			impl_.insert(k, std::make_pair(v, deletor));
			++size_;
		}

		// Obtain value of the cached function for k 
		std::pair<value_type, bool> get(const key_type& k)
		{
			auto_lock lock(mutex_);

			auto has = impl_.exsit(k); 
			if( !has.first ) 
			{
				static value_type tmp;
				return std::pair<value_type, bool>(tmp, false);
			} 
			else 
			{ 
				return impl_.update(k, has.second);
			} 
		} 


		// 清空所有数据
		void clear()
		{
			auto_lock lock(mutex_);
			impl_.clear();

			size_ = 0;
		}

		size_t capacity() const
		{
			auto_lock lock(mutex_);
			return capacity_;
		}

		size_t size() const
		{
			auto_lock lock(mutex_);
			return size_;
		}
	};


	using detail::lru_t;
	using detail::lfu_t;
	using detail::fifo_t;
}


#endif
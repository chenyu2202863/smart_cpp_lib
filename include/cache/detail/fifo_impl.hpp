#ifndef __CACHE_FIFO_IMPL_HPP
#define __CACHE_FIFO_IMPL_HPP



#include <cassert> 
#include <vector> 
#include <unordered_map>

#include "../memory_pool/sgi_memory_pool.hpp"
#include "../memory_pool/fixed_memory_pool.hpp"
#include "../extend_stl/allocator/container_allocator.hpp"


namespace cache
{
	namespace detail
	{

		// FIFO

		template< 
			typename K, 
			typename V
		>
		class fifo_t
		{
		public: 
			typedef K cache_key_type; 
			typedef V cache_value_type; 
			typedef typename cache_value_type::first_type	value_type;
			typedef typename cache_value_type::second_type	deletor_type;


			// list pool
			typedef memory_pool::st_memory_pool list_pool;

			typedef stdex::allocator::pool_allocator_t<
				cache_key_type,
				list_pool
			> list_allocator;

			// cache list
			typedef std::vector<cache_key_type, list_allocator> cache_list;


			typedef std::pair<const cache_key_type, cache_value_type> map_value_type;

			// map node pool 
			typedef memory_pool::sgi_memory_pool_t<
				false, 
				256
			> map_pool;

			// map allocator
			typedef stdex::allocator::pool_allocator_t<
				map_value_type, 
				map_pool
			> map_allocator;

			// Key to value and key history iterator 
			typedef std::unordered_map< 
				cache_key_type, 
				cache_value_type,
				std::tr1::hash<cache_key_type>,
				std::equal_to<cache_key_type>,
				map_allocator
			> cache_map;

			typedef typename cache_list::iterator		list_iterator;
			typedef typename cache_list::const_iterator	list_const_iterator;
			typedef typename cache_map::iterator		map_iterator;
			typedef typename cache_map::const_iterator	map_const_iterator;

		private:
			list_pool list_pool_;
			// Key access history 
			cache_list list_;

			map_pool map_pool_;
			// Key-to-value lookup 
			cache_map map_; 


			size_t insertIndex_;
			size_t eraseIndex_;
		public:
			// Constuctor specifies the cached function and 
			// the maximum number of records to be stored 
			explicit fifo_t(size_t c) 
				: list_(list_allocator(list_pool_))
				, map_(c, std::hash<cache_key_type>(), std::equal_to<cache_key_type>(), map_allocator(map_pool_))
				, insertIndex_(0)
				, eraseIndex_(0)
			{ 
				list_.resize(c);
				assert(c != 0);
			}


		public:
			map_iterator begin()
			{
				return map_.begin();
			}

			map_const_iterator begin() const
			{
				return map_.begin();
			}

			map_iterator end()
			{
				return map_.end();
			}

			map_const_iterator end() const
			{
				return map_.end();
			}

			// 清空所有数据
			void clear()
			{
				list_.clear();
				map_.clear();
			}

			std::pair<bool, map_iterator> exsit(const cache_key_type& k)
			{
				// Attempt to find existing record 
				auto iter = map_.find(k);
				return std::make_pair(iter != map_.end(), iter); 
			}

			// Obtain value of the cached function for k 
			std::pair<value_type, bool> update(const cache_key_type& k, const map_iterator &it)
			{
				assert(exsit(k).first);

				// Return the retrieved value 
				return std::pair<value_type, bool>(it->second.first, true); 
			} 

			void insert(const cache_key_type &key, const cache_value_type &val)
			{
				// Method is only called on cache misses 
				assert(map_.find(key) == map_.end()); 

				if( insertIndex_ >= list_.size() )
					insertIndex_ = 0;

				list_[insertIndex_++] = key;
				// Create the key-value entry, linked to the usage record. 
				map_.insert(std::make_pair(key, val));
			}

			// Purge the least-recently-used element in the cache 
			void evict() 
			{ 
				// Assert method is never called when cache is empty 
				assert(!list_.empty());
				if( list_.empty() )
					return;

				if( eraseIndex_ >= list_.size() )
					eraseIndex_ = 0;

				const cache_key_type &key = list_[eraseIndex_++];
				map_const_iterator iter = map_.find(key);
				assert(iter != map_.end());

				if( iter->second.second != 0 )
					iter->second.second(key, iter->second.first);

				map_.erase(iter);
			}

		};
	}
}





#endif
#ifndef __CACHE_LRU_IMPL_HPP
#define __CACHE_LRU_IMPL_HPP


#include <cassert> 
#include <list> 
#include <unordered_map>

#include "../memory_pool/sgi_memory_pool.hpp"
#include "../extend_stl/allocator/container_allocator.hpp"


namespace cache
{
	namespace detail
	{

		// LRU Alogrithm

		template< 
			typename K, 
			typename V
		>
		class lru_t
		{ 
		public: 
			typedef K cache_key_type; 
			typedef V cache_value_type; 

			typedef typename cache_value_type::first_type value_type;
			typedef typename cache_value_type::second_type deletor_type;

			// list node pool
			typedef memory_pool::sgi_memory_pool_t<
				false,
				sizeof(cache_key_type) + 12
			> list_pool;

			// list allocator
			typedef stdex::allocator::pool_allocator_t<
				cache_key_type, 
				list_pool
			> list_allocator;

			// Key access history, most recent at back 
			typedef std::list<
				cache_key_type, 
				list_allocator
			> cache_list; 

			// map value_type
			typedef std::pair<
				cache_value_type, 
				typename cache_list::iterator
			> mapped_type;

			typedef std::pair<cache_key_type, mapped_type> map_value_type;

			// map node pool 
			typedef memory_pool::sgi_memory_pool_t<
				false, 
				2048
			> map_pool;

			// map allocator
			typedef stdex::allocator::pool_allocator_t<
				map_value_type, 
				map_pool
			> map_allocator;

			// Key to value and key history iterator 
			typedef std::unordered_map< 
				cache_key_type, 
				mapped_type, 
				std::hash<cache_key_type>, 
				std::equal_to<cache_key_type>, 
				map_allocator
			> cache_map;

			typedef typename cache_list::iterator		list_iterator;
			typedef typename cache_list::const_iterator	list_const_iterator;
			typedef typename cache_map::iterator		map_iterator;
			typedef typename cache_map::const_iterator	map_const_iterator;

		private:
			list_pool listPool_;
			// Key access history 
			cache_list list_;


			map_pool mapPool_;
			// Key-to-value lookup 
			cache_map map_; 

		public:
			// Constuctor specifies the cached function and 
			// the maximum number of records to be stored 
			explicit lru_t(size_t c) 
				: list_(list_allocator(listPool_))
				, map_(c, std::hash<cache_key_type>(), std::equal_to<cache_key_type>(), map_allocator(mapPool_))
			{ 
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
				map_iterator iter = map_.find(k);
				return std::make_pair(iter != map_.end(), iter); 
			}

			// Obtain value of the cached function for k 
			std::pair<value_type, bool> update(const cache_key_type& k, const map_iterator &it)
			{
				assert(exsit(k).first);

				// Update access record by moving accessed key to back of list 
				list_.splice(list_.end(), list_, (*it).second.second); 

				// Return the retrieved value 
				return std::pair<value_type, bool>((*it).second.first.first, true); 
			} 

			void insert(const cache_key_type &key, const cache_value_type &val)
			{
				// Record k as most-recently-used key 
				list_iterator it = list_.insert(list_.end(), key); 

				// Create the key-value entry, linked to the usage record. 
				map_.insert(std::make_pair(key, std::make_pair(val, it))); 
			}

			// Purge the least-recently-used element in the cache 
			void evict() 
			{ 
				// Assert method is never called when cache is empty 
				assert(!list_.empty());
				if( list_.empty() )
					return;

				// Identify least recently used key 
				map_const_iterator it = map_.find(list_.front()); 
				assert(it != map_.end());
				if( it == map_.end() )
					return;

				if( it->second.first.second != 0 )
					it->second.first.second(list_.front(), it->second.first.first);

				// Erase both elements to completely purge record 
				map_.erase(it); 
				list_.pop_front();
			}
		}; 
	}
}


#endif
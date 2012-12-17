#ifndef __CACHE_LFU_IMPL_HPP
#define __CACHE_LFU_IMPL_HPP



#include <cassert> 
#include <list> 
#include <unordered_map>

#include "../memory_pool/sgi_memory_pool.hpp"
#include "../extend_stl/allocator/container_allocator.hpp"


namespace cache
{
	namespace detail
	{

		// LFU

		template< 
			typename K, 
			typename V
		>
		class lfu_t
		{
		public: 
			typedef K cache_key_type; 
			typedef V cache_value_type; 
			typedef typename cache_value_type::first_type	value_type;
			typedef typename cache_value_type::second_type	deletor_type;

			struct list_cache;

			// list cache pool
			typedef memory_pool::sgi_memory_pool_t<
				false,
				/*sizeof(list_cache)*/32 + 12
			> list_cache_pool;

			// list cache allocator
			typedef stdex::allocator::pool_allocator_t<
				list_cache, 
				list_cache_pool
			> list_cache_allocator;

			// list cache
			typedef std::list<
				list_cache, 
				list_cache_allocator
			> cache_list;

			// cache node
			struct cache_node
			{
				cache_key_type key_;
				typename cache_list::iterator parent_;
			};

			// cache node list pool
			typedef memory_pool::sgi_memory_pool_t<
				false,
				sizeof(cache_node) + 12
			> cache_node_pool;

			// cache node list allocator
			typedef stdex::allocator::pool_allocator_t<
				cache_node, 
				cache_node_pool
			> cache_node_allocator;

			// cache node list
			typedef std::list<
				cache_node,
				cache_node_allocator
			> cache_items;

			// Key access history
			struct list_cache
			{
				unsigned __int64 cnt_;
				cache_items items_;

				list_cache()
					: items_(cache_node_allocator())
				{}
			};

			// map mapped_type
			typedef std::pair<cache_value_type, typename cache_items::const_iterator> mapped_type;

			typedef std::pair<const cache_key_type, mapped_type> map_value_type;

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
			list_cache_pool listCachePool_;

			// Key access history 
			cache_list list_;

			map_pool mapPool_;

			// Key-to-value lookup 
			cache_map map_; 

		public:
			// Constuctor specifies the cached function and 
			// the maximum number of records to be stored 
			explicit lfu_t(size_t c) 
				: list_(list_cache_allocator(listCachePool_))
				, map_(c, std::tr1::hash<cache_key_type>(), std::equal_to<cache_key_type>(), map_allocator(mapPool_))
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

			std::pair<bool, map_iterator>  exsit(const cache_key_type& k)
			{
				// Attempt to find existing record 
				map_iterator iter = map_.find(k);
				return std::make_pair(iter != map_.end(), iter); 
			}

			// Obtain value of the cached function for k 
			std::pair<value_type, bool> update(const cache_key_type& k, const map_iterator &it)
			{
				assert(exsit(k).first);


				// Update access record 
				cache_list::const_iterator tmp = it->second.second->parent_;
				const cache_items::const_iterator &nodeIter = it->second.second;
				const list_iterator &iter = nodeIter->parent_;
				list_iterator next(iter);
				++next;

				if( next == list_.end() || 
					next->cnt_ != iter->cnt_ + 1 )
				{
					list_cache listNode;
					listNode.cnt_ = iter->cnt_ + 1;
					list_iterator pos = list_.insert(next, listNode);

					pos->items_.splice(pos->items_.end(), iter->items_, nodeIter);
					pos->items_.begin()->parent_ = pos;

					it->second.second = pos->items_.begin();
				}
				else
				{
					next->items_.splice(next->items_.begin(), iter->items_, nodeIter);
					next->items_.begin()->parent_ = next;

					it->second.second = next->items_.begin();
				}

				if( tmp->items_.empty() )
					list_.erase(tmp);

				// Return the retrieved value 
				return std::pair<value_type, bool>(it->second.first.first, true); 
			} 

			void insert(const cache_key_type &key, const cache_value_type &val)
			{
				// Method is only called on cache misses 
				assert(map_.find(key) == map_.end()); 

				cache_items::const_iterator iter;
				if( list_.empty() ||
					list_.front().cnt_ != 1 )
				{
					list_cache listNode;
					listNode.cnt_ = 1;
					list_iterator pos = list_.insert(list_.begin(), listNode);

					cache_node node;
					node.key_ = key;
					node.parent_ = pos;

					pos->items_.push_front(node);
					iter = pos->items_.begin();
				}
				else	
				{
					cache_node node;
					node.key_ = key;
					node.parent_ = list_.begin();

					cache_items &items = list_.front().items_;
					iter = items.insert(items.begin(), node);
				}

				// Create the key-value entry, linked to the usage record. 
				map_.insert(std::make_pair(key, std::make_pair(val, iter))); 
			}

			// Purge the least-recently-used element in the cache 
			void evict() 
			{ 
				// Assert method is never called when cache is empty 
				assert(!list_.empty());
				if( list_.empty() )
					return;

				const cache_key_type &key = list_.front().items_.front().key_;

				map_const_iterator iter = map_.find(key);
				assert(iter != map_.end());

				if( iter->second.first.second != 0 )
					iter->second.first.second(key, iter->second.first.first);

				map_.erase(iter);

				list_.front().items_.pop_front();

				if( list_.front().items_.empty() )
					list_.pop_front();
			}

		};
	}
}




#endif
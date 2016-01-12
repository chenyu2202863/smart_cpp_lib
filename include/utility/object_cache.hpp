#ifndef __UTILITY_OBJECT_CACHE_HPP
#define __UTILITY_OBJECT_CACHE_HPP


#include <unordered_map>
#include "../multithread/tls.hpp"


namespace utility {

	template < 
		typename KeyT, 
		typename ValueT, 
		typename AllocatorT = std::allocator<std::pair<const KeyT, ValueT>> 
	>
	struct object_cache_t
	{
		using container_t = std::unordered_multimap<KeyT, ValueT, std::hash<KeyT>, std::equal_to<KeyT>, AllocatorT>;
		using tls_container_t = multi_thread::tls_ptr_t<container_t>;

		tls_container_t container_;	// no way to erase

		object_cache_t() = default;
		~object_cache_t() = default;

		object_cache_t(const object_cache_t &) = delete;
		object_cache_t &operator=(const object_cache_t &) = delete;

		void insert(KeyT &&key, ValueT &&value)
		{
			if( !container_ )
				container_ = new container_t();

			container_->emplace(std::move(key), std::move(value));
		}

		std::pair<bool, ValueT> get(const KeyT &key)
		{
			if( !container_ )
				container_ = new container_t();

			auto ret = container_->equal_range(key);
			if( ret.first == ret.second )
				return std::make_pair(false, ValueT());
			else
			{
				ValueT val = std::move(ret.first->second);
				container_->erase(ret.first);
				return { true, val };
			}
		}
	};
} 

#endif
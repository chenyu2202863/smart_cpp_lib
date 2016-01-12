#ifndef __UTILITY_TIME_WHEEL_HPP
#define __UTILITY_TIME_WHEEL_HPP

#include <memory>
#include "../extend_stl/container/sync_container.hpp"
#include "circular_buffer.hpp"

namespace utility {

	template < 
		typename EntryT, 
		std::uint32_t N, 
		typename AllocatorT
	>
	struct timewheel_t
	{
		typedef EntryT entry_t;
		typedef std::shared_ptr<entry_t> entry_ptr;
		typedef std::weak_ptr<entry_t> entry_weak_ptr;

		typedef stdex::container::sync_assoc_container_t<
			entry_ptr, 
			entry_ptr,
			std::unordered_map<entry_ptr, entry_ptr>
		> entry_buckets_t;

		typedef utility::cicular_buffer_t<entry_buckets_t, N> entry_circular_buffer_t;

		entry_circular_buffer_t buffers_;

		void update_entry(const entry_ptr &entry)
		{
			(*buffers_.back())[entry] = entry;
		}

		template < typename AllocatorT = std::allocator<char> >
		void push_new_entry(const AllocatorT &allocator = AllocatorT())
		{
			typedef typename AllocatorT::rebind<entry_buckets_t>::other entry_buckets_allocator_t;
			buffers_.push_back(std::allocate_shared<entry_buckets_t>(entry_buckets_allocator_t()));
		}
	};
}

#endif
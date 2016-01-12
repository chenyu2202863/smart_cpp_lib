#ifndef __DB_PROTOBUF_CONNECTION_POOL_HPP
#define __DB_PROTOBUF_CONNECTION_POOL_HPP


#include <memory>
#include <mutex>
#include "db.hpp"
#include "../../../utility/object_pool.hpp"

namespace utility {

	typedef std::pair<bool, async::proto::detail::connection_ptr> connection_info_t;

	template < typename AllocatorT >
	struct object_pool_traits_t<std::list<std::shared_ptr<connection_info_t>, AllocatorT>>
	{
		typedef std::shared_ptr<connection_info_t> value_t;
		static std::mutex mutex_;

		static value_t pop(std::list<value_t, AllocatorT> &l)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			if( l.empty() )
			{
				return value_t();
			}
			else
			{
				value_t val = l.front();
				l.pop_front();
				return val;
			}
		}

		static void push(std::list<value_t, AllocatorT> &l, value_t && val)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			if( val->first )
				l.push_back(std::move(val));
		}

		template < typename HandlerT >
		static void for_each(std::list<value_t, AllocatorT> &l, const HandlerT &handler)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			std::for_each(l.cbegin(), l.cend(), handler);
		}

		static std::uint32_t size(std::list<value_t, AllocatorT> &l)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return l.size();
		}
	};

	template < typename AllocatorT >
	std::mutex object_pool_traits_t<std::list<std::shared_ptr<connection_info_t>, AllocatorT>>::mutex_;

}


namespace async { namespace proto { namespace detail {
	
	using utility::connection_info_t;
	typedef utility::object_pool_t<connection_info_t> connection_pool_t;
}
}
}

#endif
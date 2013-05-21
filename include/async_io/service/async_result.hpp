#ifndef __ASYNC_SERVICE_ASYNC_RESULT_HPP
#define __ASYNC_SERVICE_ASYNC_RESULT_HPP



#include <functional>
#include <type_traits>
#include <system_error>
#include <memory>
#include <cstdint>

#include "../basic.hpp"
#include "object_factory.hpp"


namespace async { namespace service {


	extern std::_Ph<1> _Error;
	extern std::_Ph<2> _Size;



	// Allocate IO Callback

	struct async_callback_base_t;

	template < typename HandlerT >
	async_callback_base_t *async_result_allocate(HandlerT &&handler);
	
	struct async_result_deallocate_t 
	{
		void operator()(async_callback_base_t *p);
	};

	typedef std::unique_ptr< async_callback_base_t, async_result_deallocate_t > async_callback_base_ptr;



	//---------------------------------------------------------------------------
	// struct async_callback_base

	struct async_callback_base_t
		: public OVERLAPPED
	{
		async_callback_base_t()
		{
			std::memset(static_cast<OVERLAPPED *>(this), 0, sizeof(OVERLAPPED));
		}

		virtual ~async_callback_base_t() {}
		virtual void invoke(std::error_code error, std::uint32_t size) = 0;
		virtual void deallocate() = 0;

	private:
		async_callback_base_t(const async_callback_base_t &);
		async_callback_base_t &operator=(const async_callback_base_t &);
	};

	template < typename OverlappedT >
	void call(OverlappedT *overlapped, std::uint32_t size, std::error_code error)
	{
		async_callback_base_ptr p(static_cast<async_callback_base_t *>(overlapped));

		p->invoke(error, size);
	}


	template < typename HandlerT, typename PoolT >
	struct win_async_callback_t
		: async_callback_base_t
	{
		HandlerT handler_;
		PoolT &pool_;

		explicit win_async_callback_t(HandlerT &&callback, PoolT &pool)
			: handler_(std::move(callback))
			, pool_(pool)
		{}

		virtual ~win_async_callback_t()
		{}

		virtual void invoke(std::error_code error, std::uint32_t size)
		{
			handler_(std::cref(error), size);
		}

		virtual void deallocate()
		{
			details::object_deallocate(this, pool_);
		}
	};


	inline void async_result_deallocate_t::operator()(async_callback_base_t *p)
	{
		p->deallocate();
	}


	template < typename HandlerT, typename PoolT >
	async_callback_base_t *async_result_allocate(HandlerT &&handler, PoolT &pool)
	{
		typedef win_async_callback_t<HandlerT, PoolT> async_callback_t;

		return reinterpret_cast<async_callback_base_t *>(
			details::object_allocate<async_callback_t>(pool, std::forward<HandlerT>(handler), pool));
	}

	struct async_callback_pool_t
	{
		typedef std::function<void *(std::uint32_t)> allocate_t;
		typedef std::function<void(void *, std::uint32_t)> deallocate_t;

		allocate_t allocate_;
		deallocate_t deallocate_;

		default_pool_t pool_;

		async_callback_pool_t();

		void *allocate(std::uint32_t sz)
		{
			return allocate_(sz);
		}

		void deallocate(void *p, std::uint32_t sz)
		{
			return deallocate_(p, sz);
		}

		void register_handler(const allocate_t &allocate, const deallocate_t &deallocate);
	};

	async_callback_pool_t &async_callback_pool_instance();


	template < typename HandlerT >
	async_callback_base_t *make_async_callback(HandlerT &&handler)
	{
		return async_result_allocate(std::forward<HandlerT>(handler), async_callback_pool_instance());
	}
}
}


#endif
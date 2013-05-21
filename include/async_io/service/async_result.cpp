#include "async_result.hpp"

namespace async { namespace service {


	std::_Ph<1> _Error;
	std::_Ph<2> _Size;


	async_callback_pool_t::async_callback_pool_t()
	{
		using namespace std::placeholders;
		allocate_ = std::bind(&default_pool_t::allocate, std::ref(pool_), _1);
		deallocate_ = std::bind(&default_pool_t::deallocate, std::ref(pool_), _1, _2);
	}


	void async_callback_pool_t::register_handler(const allocate_t &allocate, const deallocate_t &deallocate)
	{
		allocate_ = allocate;
		deallocate_ = deallocate;
	}

	async_callback_pool_t &async_callback_pool_instance()
	{
		static async_callback_pool_t pool;
		return pool;
	}


}}
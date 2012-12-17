#include "unit_test/tut/tut/tut.hpp"

#include "memory_pool/sgi_memory_pool.hpp"
#include "memory_pool/fixed_memory_pool.hpp"


namespace tut
{
	struct sgi_memory_pool
	{};

	typedef test_group<sgi_memory_pool> tf;
	typedef tf::object object;
	tf sgi_memory_pool_group("sgi_memory_pool");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("sgi_memory_pool");

		memory_pool::mt_memory_pool pool;
		void *p = pool.allocate(10);

		memcpy(p, "123", 10);

		std::cout << __FUNCTION__ << std::endl;
		
		pool.deallocate(p, 10);
	}

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("fixed_memory_pool");

		memory_pool::fixed_memory_pool_t<false, 1024> pool;
		void *p = pool.allocate(0);

		memcpy(p, "123", 1024);

		pool.deallocate(p, 0);
	}



}
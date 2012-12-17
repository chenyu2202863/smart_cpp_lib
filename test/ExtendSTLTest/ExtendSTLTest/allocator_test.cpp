#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/allocator/allocators.hpp"
#include "extend_stl/allocator/container_allocator.hpp"


namespace tut
{
	struct allocator_test
	{};

	typedef test_group<allocator_test> tf;
	typedef tf::object object;
	tf allocator_test_group("allocator_test");

	template<>template<>
	void object::test<1>()
	{
		set_test_name("allocator_unbounded");

		std::vector<int, stdex::allocator::allocator_unbounded<int>> vec;
		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);
		vec.push_back(40);
		vec.push_back(50);

		ensure_equals(vec[0], 10);
		ensure_equals(vec[1], 20);
		ensure_equals(vec[2], 30);
		ensure_equals(vec[3], 40);
		ensure_equals(vec[4], 50);
	}

	template<>template<>
	void object::test<2>()
	{
		set_test_name("allocator_fixed_size");

		std::vector<int, stdex::allocator::allocator_fixed_size<int>> vec;
		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);
		vec.push_back(40);
		vec.push_back(50);

		ensure_equals(vec[0], 10);
		ensure_equals(vec[1], 20);
		ensure_equals(vec[2], 30);
		ensure_equals(vec[3], 40);
		ensure_equals(vec[4], 50);
	}

	template<>template<>
	void object::test<3>()
	{
		set_test_name("allocator_suballoc");

		std::vector<int, stdex::allocator::allocator_suballoc<int>> vec;
		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);
		vec.push_back(40);
		vec.push_back(50);

		ensure_equals(vec[0], 10);
		ensure_equals(vec[1], 20);
		ensure_equals(vec[2], 30);
		ensure_equals(vec[3], 40);
		ensure_equals(vec[4], 50);
	}

	template<>template<>
	void object::test<4>()
	{
		set_test_name("AllocatorChunklist");

		std::vector<int, stdex::allocator::allocator_chunklist<int>> vec;
		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);
		vec.push_back(40);
		vec.push_back(50);

		ensure_equals(vec[0], 10);
		ensure_equals(vec[1], 20);
		ensure_equals(vec[2], 30);
		ensure_equals(vec[3], 40);
		ensure_equals(vec[4], 50);
	}


	template<>template<>
	void object::test<5>()
	{
		set_test_name("AllocateSyncNone");

		std::vector<int, stdex::allocator::allocator_none_sync_chunklist<int>> vec;
		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);
		vec.push_back(40);
		vec.push_back(50);

		ensure_equals(vec[0], 10);
		ensure_equals(vec[1], 20);
		ensure_equals(vec[2], 30);
		ensure_equals(vec[3], 40);
		ensure_equals(vec[4], 50);
	}

	template<>template<>
	void object::test<6>()
	{
		set_test_name("AllocatorSyncPerContainer");

		std::vector<int, stdex::allocator::allocator_per_container_newdel<int>> vec;
		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);
		vec.push_back(40);
		vec.push_back(50);

		ensure_equals(vec[0], 10);
		ensure_equals(vec[1], 20);
		ensure_equals(vec[2], 30);
		ensure_equals(vec[3], 40);
		ensure_equals(vec[4], 50);
	}

	template<>template<>
	void object::test<7>()
	{
		set_test_name("AllocatorSyncPerThread");

		std::vector<int, stdex::allocator::allocator_per_thread_suballoc<int>> vec;
		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);
		vec.push_back(40);
		vec.push_back(50);

		ensure_equals(vec[0], 10);
		ensure_equals(vec[1], 20);
		ensure_equals(vec[2], 30);
		ensure_equals(vec[3], 40);
		ensure_equals(vec[4], 50);
	}

}
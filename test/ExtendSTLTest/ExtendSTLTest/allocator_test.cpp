#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/allocator/allocators.hpp"
#include "extend_stl/allocator/container_allocator.hpp"
#include "extend_stl/allocator/stack_allocator.hpp"

#include <forward_list>

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


	template<>template<>
	void object::test<8>()
	{
		set_test_name("stack allocator");

		struct custom_t
		{
			int n_;
			custom_t(int n)
				: n_(n)
			{}
			~custom_t()
			{
			}
		};

		{
			stdex::allocator::stack_storage<1024> storage;
			stdex::allocator::stack_allocator_t<custom_t, 1024> alloc(storage);
			std::vector<custom_t, stdex::allocator::stack_allocator_t<custom_t, 1024>> vec(alloc);
			vec.emplace_back(10);
			vec.emplace_back(20);
			vec.emplace_back(30);
			vec.emplace_back(40);
			vec.emplace_back(30);
			vec.emplace_back(40);
			vec.emplace_back(30);
			vec.emplace_back(40);
			vec.push_back(50);
		}


		std::forward_list<int> n;
		n.push_front(1);
		n.push_front(2);
		n.push_front(3);

		std::forward_list<int> n2;
		n2.push_front(11);
		n2.push_front(22);
		n2.push_front(33);

		auto iter = n2.begin();
		std::advance(iter, 1);
		n.splice_after(n.before_begin(), n2, iter, n2.end());
	}
}
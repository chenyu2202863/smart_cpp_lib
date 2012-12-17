#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/atomic.hpp"


namespace tut
{
	struct atomic_test
	{};

	typedef test_group<atomic_test> tf;
	typedef tf::object object;
	tf atomic_test_group("atomic_test");

	template<>template<>
	void object::test<1>()
	{
		std::atomic<int> num;
		num = 10;
		++num;

		ensure(11 == num);
	}

}
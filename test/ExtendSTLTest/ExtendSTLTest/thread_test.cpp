#include "unit_test/tut/tut/tut.hpp"

//#include "extend_stl/thread.hpp"

#pragma message("=========to do thread==============")

namespace tut
{
	struct thread_test
	{};

	typedef test_group<thread_test> tf;
	typedef tf::object object;
	tf thread_test_group("thread_test");

	template<>template<>
	void object::test<1>()
	{
		set_test_name("thread");

		//stdex::thread thr([](){std::cout << __FUNCTION__ << std::endl;});
		//thr.join();
	}

	template<>template<>
	void object::test<2>()
	{
		set_test_name("thread_group");

		//stdex::thread_group group;
		//group.create_thread([](){std::cout << __FUNCTION__ << std::endl;});

		//group.join_all();
	}

}
#include "unit_test/tut/tut/tut.hpp"

#include <type_traits>
#include "multi_thread/system_thread_pool.hpp"


namespace tut
{
	struct system_threadpool
	{};

	typedef test_group<system_threadpool> tf;
	typedef tf::object object;
	tf tls_group("system_threadpool");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("queue_work_item");

		multi_thread::queue_work_item_pool pool([](BOOL){std::cout << "queue_work_item" << std::endl;});
		pool.call();

		::Sleep(1000);	// 模拟等待事件
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("queue_timer_pool");
	
		multi_thread::queue_timer_pool pool([](BOOL timeout){std::cout << "queue_timer_pool" << std::endl;});
		pool.call(1000, 1000);

		::Sleep(1000);
	}

	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("io_pool");


	}

	template<>
	template<>
	void object::test<4>()
	{
		set_test_name("wait_object_pool");

		//multi_thread::wait_object_pool pool();
	}

}
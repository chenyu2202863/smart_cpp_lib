#include "unit_test/tut/tut/tut.hpp"

#include <type_traits>
#include "multi_thread/thread_pool/thread_pool.hpp"
#include "multi_thread/thread_pool/task_adaptor.hpp"

namespace tut
{
	struct thread_pool
	{};

	typedef test_group<thread_pool> tf;
	typedef tf::object object;
	tf thread_pool_group("thread_pool");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("thread_pool");

		multi_thread::threadpool::thread_pool pool;

		auto size = pool.size();
		ensure(size != 0);

		pool.call([](){std::cout << "task one" << std::endl;});
		pool.call([](){std::cout << "task two" << std::endl;});

		multi_thread::event_t event_;
		event_.create(0, TRUE);
		pool.call(multi_thread::threadpool::looped_task_function([](){ std::cout << "task loop" << std::endl;}, event_, 1000));
		
		for(size_t i = 0; i != 10; ++i)
			pool.call([](){std::cout << "task one" << std::endl;});
		
		system("pause");
		event_.set_event();

		pool.active_size();

		ensure("empty", pool.empty());
		pool.shutdown();
	}


	

}
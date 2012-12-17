#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/container/blocking_queue.hpp"
#include "extend_stl/container/bounded_block_queue.hpp"

#include "multi_thread/thread.hpp"

namespace tut
{
	struct block_queue
	{};

	typedef test_group<block_queue> tf;
	typedef tf::object object;
	tf block_queue_group("block_queue");





	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("block_queue");

		stdex::container::blocking_queue_t<int> queue;

		multi_thread::thread_impl_ex consumer1, consumer2, productor;
		consumer1.register_callback([&consumer1, &queue]()->DWORD
		{
			while(!consumer1.is_aborted())
			{
				int n = queue.get();
				std::cout << n << std::endl;

				::Sleep(100);
			}

			return 0;
		});
		consumer2.register_callback([&consumer2, &queue]()->DWORD
		{
			while(!consumer2.is_aborted())
			{
				int n = queue.get();
				std::cout << n << std::endl;

				::Sleep(100);
			}

			return 0;
		});
		productor.register_callback([&productor, &queue]()->DWORD
		{
			int n = 0;

			while(!productor.is_aborted())
			{
				++n;
				queue.put(n);
				::Sleep(500);
			}

			return 0;
		});

		consumer1.start();
		consumer2.start();
		productor.start();

		Sleep(5 * 1000);

		consumer1.stop();
		consumer2.stop();
		productor.stop();
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("bouned_block_queue");

		stdex::container::bounded_block_queue_t<int> queue(10);

		multi_thread::thread_impl_ex consumer1, consumer2, productor;
		consumer1.register_callback([&consumer1, &queue]()->DWORD
		{
			while(!consumer1.is_aborted())
			{
				int n = queue.get();
				std::cout << n << std::endl;

				::Sleep(500);
			}

			return 0;
		});
		consumer2.register_callback([&consumer2, &queue]()->DWORD
		{
			while(!consumer2.is_aborted())
			{
				int n = queue.get();
				std::cout << n << std::endl;

				::Sleep(300);
			}

			return 0;
		});
		productor.register_callback([&productor, &queue]()->DWORD
		{
			int n = 0;

			while(!productor.is_aborted())
			{
				++n;
				queue.put(n);
				::Sleep(100);
			}

			return 0;
		});

		consumer1.start();
		consumer2.start();
		productor.start();

		Sleep(5 * 1000);

		consumer1.stop();
		consumer2.stop();

		queue.get();
		productor.stop();

		
	}
}
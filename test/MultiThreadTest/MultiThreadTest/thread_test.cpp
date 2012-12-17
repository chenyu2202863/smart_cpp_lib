#include "unit_test/tut/tut/tut.hpp"

#include "multi_thread/thread.hpp"


namespace tut
{
	struct multi_thread_test
	{};

	typedef test_group<multi_thread_test> tf;
	typedef tf::object object;
	tf multi_thread_group("multi_thread");


	DWORD thread_handler(multi_thread::thread_impl_ex &thr)
	{
		while(!thr.is_aborted())
		{
			std::cout << "+";
			::Sleep(200);
		}

		return 0;
	}

	template < typename stop_policy >
	DWORD thread_handler3(multi_thread::thread_implex_t<stop_policy> &thr)
	{
		while(!thr.is_aborted())
		{
			std::cout << "*";
			::Sleep(300);
		}

		return 0;
	}


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("multi_thread");

		multi_thread::thread_impl_ex thread1;
		thread1.register_callback([&thread1]()->DWORD
		{
			while(!thread1.is_aborted())
			{
				std::cout << "-";
				::Sleep(100);
			}

			return 0;
		});

		multi_thread::thread_impl_ex thread2;
		thread2.register_callback(std::bind(&thread_handler, std::ref(thread2)));


		multi_thread::thread_impl_ex_event thread3;
		thread3.register_callback(std::bind(&thread_handler3<multi_thread::detail::stop_event>, std::ref(thread3)));

		thread1.start();
		thread2.start();
		thread3.start();

		ensure("thread1 running", thread1.is_running());
		ensure("thread1 suspend", thread1.suspend());
		ensure("thread1 is suspended", thread1.is_suspended());

		ensure("thread1 resume", thread1.resume());
		ensure("thread1 running", thread1.is_running());

		::Sleep(1000);
		thread1.stop();
		thread2.stop();
		thread3.stop();
	}

}
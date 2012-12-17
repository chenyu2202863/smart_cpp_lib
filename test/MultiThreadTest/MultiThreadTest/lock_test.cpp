#include "unit_test/tut/tut/tut.hpp"

#include <type_traits>
#include "multi_thread/lock.hpp"


namespace tut
{
	struct lock
	{};

	typedef test_group<lock> tf;
	typedef tf::object object;
	tf lock_group("lock");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("cs");

		
		typedef multi_thread::critical_section mutex;
		typedef multi_thread::auto_lock_t<mutex> auto_lock;

		mutex m;
		{
			auto_lock lock(m);
		}
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("spin lock");


		typedef multi_thread::spin_lock mutex;
		typedef multi_thread::auto_lock_t<mutex> auto_lock;

		mutex m(1000);
		{
			auto_lock lock(m);
		}
	}


	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("RW lock");

		// Vista ртио
		/*typedef multi_thread::shared_rw_lock mutex;
		typedef multi_thread::auto_lock_t<mutex> auto_lock;

		mutex m;
		{
			auto_lock lock(m);
		}*/
	}

	template<>
	template<>
	void object::test<4>()
	{
		set_test_name("mutex");


		typedef multi_thread::mutex mutex;
		typedef multi_thread::auto_lock_t<mutex> auto_lock;

		mutex m;
		{
			auto_lock lock(m);
		}
	}

	template<>
	template<>
	void object::test<5>()
	{
		set_test_name("event");

		multi_thread::event_t e;
		ensure("event create", e.create());

		ensure("event is_open", e.is_open());

		ensure("set event", e.set_event());

		ensure("event is_signalled", e.is_signalled());

		ensure("reset event", e.reset_event());

		e.wait_for_event(2000);
	}

	template<>
	template<>
	void object::test<6>()
	{
		set_test_name("condition");

		multi_thread::semaphore_condition condition;
		multi_thread::critical_section mutex;
		condition.signal();
		//ensure("condtion wait", condition.wait(mutex, 1000));
	}

	template<>
	template<>
	void object::test<7>()
	{
		set_test_name("once_init");

		multi_thread::once_init<decltype(this)>([](){ std::cout << "once_init" << std::endl; });
		multi_thread::once_init<decltype(this)>([](){ std::cout << "once_init" << std::endl; });
		multi_thread::once_init<decltype(*this)>([](){ std::cout << "once_init2" << std::endl; });
		multi_thread::once_init<decltype(*this)>([](){ std::cout << "once_init2" << std::endl; });

		struct test_type
		{
			test_type()
			{
				std::cout << __FUNCTION__ << std::endl;
			}
		};
		static std::shared_ptr<test_type> val;
		multi_thread::once_init(val);

	}
} 
#include "unit_test/tut/tut/tut.hpp"

#include <vector>
#include "log.hpp"
#include "multi_thread/thread.hpp"

namespace tut
{
	struct wx_log_test
	{};

	typedef test_group<wx_log_test> tf;
	typedef tf::object object;
	tf dwx_log_test_testgroup("wx_log_test");


	


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("wx_log_test");

		wx::log log_impl(L"C:/log", L"2014-08-01.log", 1);

		log_impl(wx::Info) << "test " << 10 << 10.1 << 'c' << wx::endl;
		log_impl(wx::Warning) << "test " << 10 << 10.1 << 'c' << wx::endl;
		log_impl(wx::Critical) << "test " << 10 << 10.1 << 'c' << wx::endl;

		log_impl.del_expired();
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("multithread log");

		wx::log log_impl(L"C:/log", L"2014-08-01.log", 1);

		std::vector<multi_thread::thread_impl_ex> threads(200, multi_thread::thread_impl_ex([&log_impl]()->DWORD
		{
			for(size_t i = 0; i != 100; ++i)
			{
				log_impl(wx::Info) << "test " << 10 << 10.1 << 'c' << "long long string long long string" << wx::endl;
				log_impl(wx::Warning) << "test " << 10 << 10.1 << 'c' << "long long string long long string" << wx::endl;
				log_impl(wx::Critical) << "test " << 10 << 10.1 << 'c' << "long long string long long string" << wx::endl;
			}

			return 0;
		}));
		
		std::for_each(threads.begin(), threads.end(), [](multi_thread::thread_impl_ex &thr)
		{
			thr.join();
		});
		

		log_impl.del_expired();
	}
}
#include "unit_test/tut/tut/tut.hpp"

#include "log/log.hpp"
#include "multi_thread/thread.hpp"

namespace tut
{
	struct log_system_test
	{};

	typedef test_group<log_system_test> tf;
	typedef tf::object object;
	tf log_system_test_testgroup("log_system_test");


	template < typename LogT >
	void Print(LogT &log)
	{
		log << log_system::level(log_system::filter::Critical) << log_system::space << 'c' << log_system::space << log_system::detail::select<typename LogT::char_type>("test1", L"test1") << log_system::endl;
		log << log_system::level(log_system::filter::Debug) << log_system::space << 'a' << log_system::space << log_system::detail::select<typename LogT::char_type>("test2", L"test2") << log_system::endl;
	}


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("log_system_test");

		log_system::file_log log("log_file_test.log");
		Print(log);

		log_system::debug_log log2;
		Print(log2);

		log_system::string_log log3;
		Print(log3);

		log_system::console_log log4;
		Print(log4);
	}

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("wchar_log_test");

		log_system::w_file_log log(L"w_log_file_test.log");
		Print(log);

		log_system::w_debug_log log2;
		Print(log2);

		log_system::w_string_log log3;
		Print(log3);

		log_system::w_console_log log4;
		Print(log4);
	}



	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("multi_test");

		log_system::multi_log_t<char> multi_log;
		log_system::multi_file_storage multi1("multi_log.log");
		log_system::multi_debug_storage multi2;
		log_system::multi_string_storage multi3;

		multi_log.add(multi1);
		multi_log.add(multi2);
		multi_log.add(multi3);

		Print(multi_log);
	}


	template<>
	template<>
	void object::test<4>()
	{
		set_test_name("cache_test");

		typedef log_system::storage::cache_storage_t<512, log_system::storage::file_storage_t<char>> file_cache_storage;
		typedef log_system::log_t<file_cache_storage, log_system::filter::default_filter> log_type;
		log_type log("cache_file.log");

		Print(log);
	}

	template<>
	template<>
	void object::test<5>()
	{
		set_test_name("async_test");

		typedef log_system::storage::async_storage_t<log_system::storage::debug_storage_t<char>> debug_async_storage;
		typedef log_system::log_t<debug_async_storage, log_system::filter::default_filter> log_type;
		log_type log;

		::Sleep(100);
		Print(log);
		::Sleep(100);
	}


	template<>
	template<>
	void object::test<6>()
	{
		set_test_name("thread safe");

		log_system::file_log log("thread_safe.log");
		//log_system::safe(log) << 1 << "cc" << log_system::endl; 
		Print(safe(log));

		log_system::console_log debug_log;
		multi_thread::thread_impl_ex thread1, thread2, thread3;
		thread1.register_callback([&debug_log]()->DWORD
		{
			size_t i = 0;
			while(++i != 100)
				debug_log << log_system::clr(log_system::format::Red) << 1 << "23" << 456 << "7890" << log_system::endl; 
			return 0;
		});
		thread2.register_callback([&debug_log]()->DWORD
		{
			size_t i = 0;
			while(++i != 100)
				debug_log << log_system::clr(log_system::format::Green) << 1 << "23" << 456 << "7890" << log_system::endl;
			return 0;
		});
		thread3.register_callback([&debug_log]()->DWORD
		{
			size_t i = 0;
			while(++i != 100)
				debug_log << log_system::clr(log_system::format::Blue) << 1 << "23" << 456 << "7890" << log_system::endl;
			return 0;
		});

		thread1.join();
		thread2.join();
		thread3.join();

	}
}
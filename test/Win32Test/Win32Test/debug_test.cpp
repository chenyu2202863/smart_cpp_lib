
#include "unit_test/tut/tut/tut.hpp"
#include "Win32/debug/mini_dump.hpp"
#include "win32/debug/stack_walker.hpp"
#include "win32/debug/memory_leak.hpp"

namespace tut
{
	struct Debug
	{};

	typedef test_group<Debug> debug_test;
	typedef debug_test::object object;
	debug_test system_group("Debug");

	win32::debug::mini_dump dump;

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("minidump");

		int n = 0;

		// 取消注释即产生dump文件
		//int m  = n / 10 / n;

	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("stack walker");

		win32::debug::dump_stack([](void *pvAddress, size_t lineNum, LPCSTR fileName, LPCSTR szModule, LPCSTR szSymbol)
		{
			std::cout << pvAddress << " " << lineNum << " "
				<< fileName << " " << szModule << " " << szSymbol << std::endl;
		},
		[](LPCSTR msg)
		{
			std::cout << msg << std::endl;
		}, 20);

	}


	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("memory leak");

		char *p1 = new char;
		char *p2 = (char *)malloc(10);
	}
}
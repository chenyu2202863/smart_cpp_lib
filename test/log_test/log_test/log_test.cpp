// log_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

// Win32Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include "unit_test/tut/tut/tut.hpp"
#include "unit_test/tut/tut/tut_console_reporter.hpp"
#include "unit_test/tut/tut/tut_main.hpp"

#include <iostream>




namespace tut
{
	test_runner_singleton runner;
}

int main(int argc, const char* argv[])
{
	tut::console_reporter reporter;
	tut::runner.get().set_callback(&reporter);

	try
	{
		if(tut::tut_main(argc, argv))
		{
			if(reporter.all_ok())
			{
				;
			}
			else
			{
				std::cerr << "\nFAILURE and EXCEPTION in these tests are FAKE ;)" << std::endl;
			}
		}
	}
	catch(const std::exception &ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;
	}


	system("pause");
	return 0;
}


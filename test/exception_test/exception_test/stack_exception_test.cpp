#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include <exception>
#include <iostream>

#include "exception/exception_base.hpp"

namespace tut
{
	struct stack_exception
	{};

	typedef test_group<stack_exception> tf;
	typedef tf::object object;
	tf base64_group("stack_exception");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("std::exception");

		std::exception_ptr ptr;

		try
		{
			try
			{
				throw std::runtime_error("123");
			}
			catch(...)
			{
				ptr = std::current_exception();
				std::rethrow_exception(ptr);
			}
		}
		catch(std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
		
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("exception::exception_base");

		try
		{
			throw exception::exception_base("exception: ") << 1 << 1.1 << 1L << '1' << "11";
		}
		catch(exception::exception_base &e)
		{
			std::cout << e.what() << std::endl;
			e.dump();
		}
	}
	



}

#include "unit_test/tut/tut/tut.hpp"

#include "utility/scope_exit.hpp"




namespace tut
{
	struct scope_exit_test
	{};

	typedef test_group<scope_exit_test> tf;
	typedef tf::object object;
	tf scope_exit_test_group("scope_exit_test");


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("scope_exit_test");

		auto val = utility::make_scope_exit([](){ std::cout << "init" << std::endl;},
			[](){std::cout << "uninit" << std::endl;});
		
		std::cout << "running..." << std::endl;

	}

}
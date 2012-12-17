
#include "unit_test/tut/tut/tut.hpp"

#include "utility/utility.hpp"


namespace tut
{
	struct utility_test
	{};

	typedef test_group<utility_test> tf;
	typedef tf::object object;
	tf utility_test_group("utility_test");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("app path");

		ensure("app path error", !utility::get_app_path().empty());
	}

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("system path");

		ensure("system path error", !utility::get_system_path().empty());
	}


	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("file version");

		ensure("file version error", utility::get_file_version(utility::get_app_path()).empty());
	}


	template<>
	template<>
	void object::test<4>()
	{
		set_test_name("compare file version");

		ensure("compare file version", utility::compare_file_version("3.1.65.48", "3.0.65.48"));
		ensure("compare file version", !utility::compare_file_version("3.1.65.48", "3.20.65.48"));
		ensure("compare file version", !utility::compare_file_version("3.11.65.48", "3.12.65.48"));
		ensure("compare file version", !utility::compare_file_version("3.10.65.48", "3.111.65.48"));

		ensure("compare file version", !utility::compare_file_version("16.4.953.8", "16.4.953.18"));
	}

	template<>
	template<>
	void object::test<5>()
	{
		set_test_name("guid string");

		ensure("guid string", utility::create_guid_string() != utility::create_guid_string());
	}
}
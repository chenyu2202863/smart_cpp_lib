
#include "unit_test/tut/tut/tut.hpp"
#include "win32/reg/registry.h"


namespace tut
{
	struct Reg
	{};

	typedef test_group<Reg> RegTest;
	typedef RegTest::object object;
	RegTest system_group("reg");


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("register");

		win32::reg::reg_key reg;
		ensure("open", reg.open(HKEY_CURRENT_USER, _T("Pos"), KEY_ALL_ACCESS));

		bool test_in_bool = false;
		bool test_out_bool = false;
		ensure("write bool", reg.write_value(_T("test_bool"), test_in_bool));
		ensure("read bool", reg.read_value(_T("test_bool"), test_out_bool));
		ensure("test_bool", test_in_bool == test_out_bool);

		char test_in_char = 'w';
		char test_out_char = 0;
		ensure("write char", reg.write_value(_T("test_char"), test_in_char));
		ensure("read char", reg.read_value(_T("test_char"), test_out_char));
		ensure("test_char", test_in_char == test_out_char);


		int test_in_int = 111;
		int test_out_int = 0;
		ensure("write int", reg.write_value(_T("test_int"), test_in_int));
		ensure("read int", reg.read_value(_T("test_int"), test_out_int));
		ensure("test_int", test_in_int == test_out_int);

		long test_in_long = 10;
		long test_out_long = 0;
		ensure("write long", reg.write_value(_T("test_long"), test_in_long));
		ensure("read long", reg.read_value(_T("test_long"), test_out_long));
		ensure("test_long", test_in_long == test_out_long);


		double test_in_double = 10.123;
		double test_out_double = 0;
		ensure("write double", reg.write_value(_T("test_double"), test_in_double));
		ensure("read double", reg.read_value(_T("test_double"), test_out_double));
		ensure("test_double", test_in_double == test_out_double);
	}


	template<>
	template<>
	void object::test<2>()
	{
		

	}

}
#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/string/algorithm.hpp"


namespace tut
{
	struct string_algorithm
	{};

	typedef test_group<string_algorithm> tf;
	typedef tf::object object;
	tf string_algorithm_group("string_algorithm");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("to upper");

		std::string val("asd");
		std::string dst = stdex::to_upper(val);
		ensure("to upper", dst == "ASD");

		std::string val2("asd");
		stdex::to_upper(val2);
		ensure("to upper", val2 == "ASD");

		std::wstring wval(L"asd");
		wval = stdex::to_upper(wval);
		ensure("to upper", wval == std::wstring(L"ASD"));

		char cval[] = "asd";
		stdex::to_upper(cval);
		ensure_equals("to upper", cval, "ASD");
	}

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("to lower");

		std::string val("ADS");
		std::string dst = stdex::to_lower(val);
		ensure("to lower", dst == "ads");

		std::wstring wval(L"ADS");
		wval = stdex::to_lower(wval);
		ensure("to lower", wval == std::wstring(L"ads"));

		char cval[] = "ADS";
		stdex::to_lower(cval);
		ensure_equals("to lower", cval, "ads");
	}


	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("to number");

		ensure_equals("to number", 10, (int)stdex::to_number("10"));
		ensure_equals("to number", 10, (int)stdex::to_number(std::string("10")));
		ensure_equals("to number", 10, (int)stdex::to_number(std::wstring(L"10")));

		double val = stdex::to_number("10.01");
		ensure_equals("to number", 10.01, val);

		bool val1 = stdex::to_number("false");
		ensure_equals("to number", false, val1);

		bool val2 = stdex::to_number("true");
		ensure_equals("to number", true, val2);

		//bool val3 = stdex::to_number("TRUE");
		//bool val4 = val3;
		//ensure_equals("to number", true, val3);
	}


	template<>
	template<>
	void object::test<4>()
	{
		set_test_name("to string");

		ensure_equals("to string", std::string("10"), (std::string)stdex::to_string(10));
		ensure_equals("to string", std::string("10"), (std::string)stdex::to_string(10));
		ensure_equals("to string", std::wstring(L"10"), (std::wstring)stdex::to_string(10));

		std::string val = stdex::to_string(10.0101, 3);
		ensure_equals("to string", std::string("10.010"), val);

		std::wstring val1 = stdex::to_string(false);
		ensure_equals("to string", std::wstring(L"false"), val1);
	}

	template<>
	template<>
	void object::test<5>()
	{
		set_test_name("trim");

		std::string val1 = "  val";
		stdex::trim_left(val1);
		ensure("trim left", val1 == "val");

		std::string val2 = "val   ";
		stdex::trim_right(val2);
		ensure("trim right", val2 == "val");

		std::string val3 = "   val   ";
		stdex::trim(val3);
		ensure("trim", val3 == "val");
	}


	template<>
	template<>
	void object::test<6>()
	{
		set_test_name("split");

		std::string val = "1  1,2,3,4,5,6,7 12,8, 9";
		std::vector<std::string> vec;
		stdex::split(vec, val, ',');

		std::vector<int> int_vec;
		stdex::split(int_vec, val, ',');

		ensure("split", "5" == stdex::split(val, ',', 4));
	}

	template<>
	template<>
	void object::test<7>()
	{
		set_test_name("compare_no_case");

		std::wstring lhs = L"abcdefg";
		std::wstring rhs = L"ABCDEFG";

		ensure("compare_no_case", stdex::compare_no_case(lhs, rhs) == 0);
	}

	template<>
	template<>
	void object::test<8>()
	{
		set_test_name("erase");

		std::wstring lhs = L"a,b,c,d,e,f,g";
		stdex::erase(lhs, L',');

		ensure("compare_no_case", lhs == L"abcdefg");
	}


	template<>
	template<>
	void object::test<9>()
	{
		set_test_name("compare compare_phonetic");

		std::wstring lhs = L"哈哈";
		std::wstring rhs = L"拉拉";

		ensure("compare_phonetic", stdex::compare_phonetic(lhs, rhs) != 0);
	}

	template<>
	template<>
	void object::test<10>()
	{
		set_test_name("replace_all");

		std::string val = "123{测试}数据123{测试}";
		std::string lhs = "{测试}";
		std::string rhs = "test";

		stdex::replace_all(val, lhs, rhs);
		ensure("replace_all", val == "123test数据123test");
	}


	template<>
	template<>
	void object::test<11>()
	{
		set_test_name("find");

		std::string lhs = "BBWWwAA";
		std::string rhs = "wwW";

		ensure("find no case", stdex::find_nocase(lhs, rhs));
	}
}
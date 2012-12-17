#include "unit_test/tut/tut/tut.hpp"

#include "utility/select.hpp"




namespace tut
{
	struct select_test
	{};

	typedef test_group<select_test> tf;
	typedef tf::object object;
	tf select_test_group("select_test");




	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("select_test");

		char a = 'a';
		wchar_t aa = L'a';
		auto val = utility::select<char>(a, aa);
		auto val2 = utility::select<char>('a', L'a');

		auto func = utility::select<char>(printf, wprintf);
		func(utility::select<char>("aa", L"aa"));
		std::cout << val << std::endl;
	}

}
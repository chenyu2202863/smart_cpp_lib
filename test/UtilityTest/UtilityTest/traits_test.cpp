#include "unit_test/tut/tut/tut.hpp"

#include "utility/traits.hpp"



namespace tut
{
	struct traits_test
	{};

	typedef test_group<traits_test> tf;
	typedef tf::object object;
	tf traits_test_group("traits_test");


	CREATE_MEMBER_DETECTOR(first);
	CREATE_MEMBER_DETECTOR(second);

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("traits_test");

		typedef std::pair<int, double> Pair;
		static_assert(utility::detect_first<Pair>::value && utility::detect_second<Pair>::value, "detect pair");
		static_assert(utility::detect_first<Pair>::value && utility::detect_second<Pair>::value, "detect pair");
	}



	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("member object pointer");


		
		
	}

}
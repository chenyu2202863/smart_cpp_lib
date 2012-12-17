#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include "algorithm/md5.hpp"


namespace tut
{
	struct Md5
	{};

	typedef test_group<Md5> tf;
	typedef tf::object object;
	tf md5_group("Md5");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("Md5");

		std::string src = "J ‘ ‘ø¥ø¥**7712&";

		auto val = algorithm::md5::encode_string(src.c_str(), src.length());

		ensure("char", !val.empty());
	}

}
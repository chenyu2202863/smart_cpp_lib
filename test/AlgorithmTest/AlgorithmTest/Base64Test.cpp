#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include "algorithm/base64.hpp"


namespace tut
{
	struct Base64
	{};

	typedef test_group<Base64> tf;
	typedef tf::object object;
	tf base64_group("Base64");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("decode");

		std::wstring src = L"J试试看看**7712&";

		std::wstring tmp = algorithm::base64::encode(src);

		std::wstring dst = algorithm::base64::decode(tmp);

		ensure("wstring", dst == src);
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("decode");

		std::string src = "JXBhdGglXMqlyP25+tOivdy0q1wqLio=";

		std::string dst = algorithm::base64::decode(src);

		std::string tmp("%path%\\圣三国英杰传\\*.*");
		ensure("string", tmp == dst);

	}




}
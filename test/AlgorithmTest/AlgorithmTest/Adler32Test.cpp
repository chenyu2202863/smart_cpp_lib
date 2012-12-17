#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include "algorithm/adler32.hpp"


namespace tut
{
	struct Adler32
	{};

	typedef test_group<Adler32> tf;
	typedef tf::object object;
	tf base64_group("Adler32");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("Adler32");

		std::string src = "J试试看看**7712&";

		std::uint32_t num = algorithm::adler::adler32(src);

		ensure("char", num != 0);
	}

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("Adler32");

		std::wstring src = L"J试试看看**7712&";

		std::uint32_t num = algorithm::adler::adler32(src);

		ensure("wchar_t", num != 0);
	}



}
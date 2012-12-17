#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include "algorithm/crc32.hpp"


namespace tut
{
	struct CRC32
	{};

	typedef test_group<CRC32> tf;
	typedef tf::object object;
	tf crc32_group("CRC32");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("crc32");

		std::vector<int> vec(10);
		std::generate(vec.begin(), vec.end(), []()->int
		{
			static int i = 0;
			return ++i;
		});

		algorithm::crc::crc32 crc;
		crc.add(vec.data(), vec.size());
		std::uint32_t dst = crc.get();

		ensure("dst != 0", dst != 0);
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("crc32");

		char buffer[] = "ccccaaaa";
		
		algorithm::crc::crc32 crc;
		crc << buffer;
		std::uint32_t dst1 = crc.get();

		algorithm::crc::crc32 crc2;
		crc2.add(buffer);
		std::uint32_t dst2 = crc2.get();

		ensure("dst1 == dst2", dst1 == dst2);
	}
	
}
#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include "algorithm/encrypt.hpp"

namespace tut
{
	struct Encrypt
	{};

	typedef test_group<Encrypt> tf;
	typedef tf::object object;
	tf encrypt_group("Encrypt");


	template<>
	template<>
	void object::test<1>()
	{
		char buf[8] = {0x11, 'a', 'b', 'c', 0x12 };
		char dest[_countof(buf)] = {0};

		const char key[] = "1234";
		char dest2[_countof(buf)] = {0};

		algorithm::crypt::tea_encipher(buf, dest, _countof(buf), key, _countof(key));
		algorithm::crypt::tea_decipher(dest, dest2, _countof(buf), key, _countof(key));
		ensure("tea", memcmp(dest2, buf, _countof(buf)) == 0);
	}
}
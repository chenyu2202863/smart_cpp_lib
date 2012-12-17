#include "unit_test/tut/tut/tut.hpp"

#include "utility/ini.hpp"


namespace tut
{
	struct ini_test
	{};

	typedef test_group<ini_test> tf;
	typedef tf::object object;
	tf ini_test_group("ini_test");

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("ini file");

		utility::ini config(L"svr.ini");

		auto sections = config.get_all_sections();

		const stdex::tString &val = config.get_val(L"svr", L"ip");
		ensure(L"192.168.1.1" == val);
		ensure(5050 == (unsigned short)config.get_val(L"svr", L"port"));

		const stdex::tString &val2 = config.get_val(L"cli", L"ip");
		ensure(L"10.34.43.1" == val2);
		ensure(5051 == (unsigned short)config.get_val(L"cli", L"port"));
	}

	

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("ini buffer");

		std::ifstream f("svr.ini");
		std::vector<char> buf;

		f.seekg(0, std::ios::end);
		size_t len = f.tellg();
		f.seekg(0, std::ios::beg);

		buf.resize(len + 1);
		f.read(&buf[0], len);
		
		utility::ini config(buf.data(), buf.size());

		const stdex::tString &val = config.get_val(L"svr", L"ip");
		ensure(L"192.168.1.1" == val);
		ensure(5050 == (unsigned short)config.get_val(L"svr", L"port"));

		const stdex::tString &val2 = config.get_val(L"cli", L"ip");
		ensure(L"10.34.43.1" == val2);
		ensure(5051 == (unsigned short)config.get_val(L"cli", L"port"));
	}

}
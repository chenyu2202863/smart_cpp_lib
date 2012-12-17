
#include "unit_test/tut/tut/tut.hpp"
#include "win32/http/win_http_helper.hpp"


namespace tut
{
	struct Http
	{};

	typedef test_group<Http> RegTest;
	typedef RegTest::object object;
	RegTest system_group("Http");


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("win_http");
		
		auto buf = win32::http::download_file(L"HTTP://10.34.41.204:204/test_file_25+test_client_25/FeeServer_1.0.0.26.exe", [](const char *, size_t sz)->bool
		{
			std::cout << ".";
			return false;
		});
		
		ensure("buffer size", buf.second != 0);
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("wininet");

		

	}

}
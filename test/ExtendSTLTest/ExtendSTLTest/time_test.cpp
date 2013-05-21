#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/time.hpp"

#include <codecvt>


namespace tut
{
	struct time_test
	{};

	typedef test_group<time_test> tf;
	typedef tf::object object;
	tf time_test_group("time_test");


	template<>template<>
	void object::test<1>()
	{
		stdex::time::system_clock::time_point start = stdex::time::system_clock::now();

		for ( long i = 0; i < 10000000; ++i )
			std::sqrt( 123.456L ); // burn some time

		stdex::time::duration<double> sec = stdex::time::system_clock::now() - start;
		std::cout << "took " << sec.count() << " seconds\n";
	}

	template <>
	template <>
	void object::test<2>()
	{
		std::string now_time = stdex::time::time_2_string<char>(stdex::time::now());
		std::cout << now_time << std::endl;

		std::wstring date_time = stdex::time::time_2_string<wchar_t>(stdex::time::now(), L"%Y-%m-%d");
		std::wcout << date_time << std::endl;

		std::wstring time_time = stdex::time::time_2_string<wchar_t>(stdex::time::now(), L"%H:%M:%S");
		std::wcout << time_time << std::endl;
	
	}

	template <>
	template <>
	void object::test<3>()
	{
		std::time_t now_time = stdex::time::string_2_time(std::string("2012-11-23 11:42:35"), "%Y:%M:%S");
		std::time_t wnow_time = stdex::time::string_2_time(std::wstring(L"2012-11-23 11:42:35"), L"%Y:%M:%S");

		std::wcout << stdex::time::time_2_string<wchar_t>(now_time) << std::endl;
		std::wcout << stdex::time::time_2_string(now_time, L"%Y-%m-%d") << std::endl;
		std::cout << stdex::time::time_2_string(now_time, "%H:%M:%S") << std::endl;
	}


	template <>
	template <>
	void object::test<4>()
	{
		typedef std::chrono::duration<int, std::ratio<1, 100000000>> shakes;
		typedef std::chrono::duration<int, std::centi> jiffies;
		typedef std::chrono::duration<float, std::ratio<12096,10000>> microfortnights;
		typedef std::chrono::duration<float, std::ratio<3155,1000>> nanocenturies;

		std::chrono::seconds sec(1);

		std::cout << "1 second is:\n";

		std::cout << std::chrono::duration_cast<shakes>(sec).count()
			<< " shakes\n";
		std::cout << std::chrono::duration_cast<jiffies>(sec).count()
			<< " jiffies\n";
		std::cout << std::chrono::duration_cast<microfortnights>(sec).count()
			<< " microfortnights\n";
		std::cout << std::chrono::duration_cast<nanocenturies>(sec).count()
			<< " nanocenturies\n";
	}

	template < >
	template < >
	void object::test<5>()
	{
		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();
		::Sleep(10);
		end = std::chrono::system_clock::now();

		auto elapsed_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);

		std::cout << "finished computation at " << std::ctime(&end_time)
			<< "elapsed time: " << elapsed_seconds << "s" << std::endl;
	}

	template <>
	template <>
	void object::test<6>()
	{
		std::chrono::time_point<std::chrono::system_clock> now;
		now = std::chrono::system_clock::now();
		std::time_t t_now = std::chrono::system_clock::to_time_t(now);

		std::time_t now_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(24));

		std::wcout << std::put_time(std::localtime(&t_now), L"%Y/%m/%d %H:%M:%S") << std::endl;
	}

	template <>
	template <>
	void object::test<7>()
	{
		std::time_t t = std::time(NULL);
		
		stdex::time::seconds sec(3600);
		stdex::time::minutes min = stdex::time::duration_cast<stdex::time::minutes>(sec);
	}

	template <>
	template <>
	void object::test<8>()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t beg = std::chrono::system_clock::to_time_t(now);
		std::time_t end = std::chrono::system_clock::to_time_t(now + std::chrono::hours(23));

		time_t dest = std::chrono::system_clock::to_time_t(now + std::chrono::hours(22));
		ensure(stdex::time::is_in_time_range(beg, end, dest));
	}

	template <>
	template <>
	void object::test<9>()
	{
		std::string now_tmp = stdex::time::time_2_string<char>(stdex::time::now(), "%H:%M:%S");
		std::time_t now = stdex::time::string_2_time(now_tmp, "%H:%M:%S");

		
	}
}
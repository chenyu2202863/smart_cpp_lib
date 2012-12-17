#include "unit_test/tut/tut/tut.hpp"

#include "win32/file_system/file_helper.hpp"
#include "win32/file_system/filesystem.hpp"


namespace tut
{
	struct File
	{};

	typedef test_group<File> tf;
	typedef tf::object object;
	tf file_group("win32::filesystem");

	const std::string path1 = "C:/test1/test1/test1";
	const std::wstring path2 = L"C:/test2/test2/test2";
	const std::wstring path3 = L"C:\\test3\\test3\\test3\\";
	const std::string path4 = "C:\\test4\\test4\\test4\\";

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("mk_directory");

		ensure("path1", win32::filesystem::mk_directory(path1));
		ensure("path2", win32::filesystem::mk_directory(path2));
		ensure("path3", win32::filesystem::mk_directory(path3));
		ensure("path4", win32::filesystem::mk_directory(path4));
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("del_directory");
		ensure("path1", win32::filesystem::del_directoy("C:/test1"));
		ensure("path2", win32::filesystem::del_directoy("C:/test2"));
		ensure("path3", win32::filesystem::del_directoy("C:/test3"));
		ensure("path4", win32::filesystem::del_directoy("C:/test4"));
	}

	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("partion_speed");
		size_t speed1 = win32::filesystem::partition_speed('C');
		size_t speed2 = win32::filesystem::partition_speed(L'c');
	}

    template<>
    template<>
    void object::test<4>()
    {
        set_test_name("combine file path");

        std::string str_path("c:\\test\\"), str_file("test.txt");
        ensure_equals("combine file path", std::string("c:\\test\\test.txt"), win32::filesystem::combine_file_path(str_path, str_file));
    }

	template<>
	template<>
	void object::test<5>()
	{
		set_test_name("check file");

        ensure(win32::filesystem::is_file_exist(std::string("c:/test.txt")));
	}
	
    template<>
    template<>
    void object::test<6>()
    {
        set_test_name("check directory");

        ensure(win32::filesystem::is_directory_exist("c:/test/"));
    }

    template<>
    template<>
    void object::test<7>()
    {
        set_test_name("get file size");

        ensure(win32::filesystem::file_size(std::string("C:\\WINDOWS\\NOTEPAD.EXE")) > 0);
    }

    template<>
    template<>
    void object::test<8>()
    {
        set_test_name("get file last write time");

        ensure(win32::filesystem::file_last_write_time(std::string("C:\\WINDOWS\\NOTEPAD.EXE")) > 0);
    }

    template<>
    template<>
    void object::test<9>()
    {
        set_test_name("check_disk_space");

        std::string info("");
        ensure(win32::filesystem::check_disk_space(std::string("c:\\"), 1024, info));
    }


	template<>
	template<>
	void object::test<10>()
	{
		set_test_name("file_system");
		
		win32::file::path p("C:\\Windows\\System"); 
		ensure_equals("string", p.string(), "C:/Windows/System");
		ensure_equals("file string", p.file_string(), "C:\\Windows\\System");
		ensure_equals("directory string", p.directory_string(), "C:\\Windows\\System"); 
		ensure_equals("root name", p.root_name(), "C:"); 
		ensure_equals("root directory", p.root_directory(), "/"); 
		ensure_equals("root path", p.root_path().string(), "C:/"); 
		ensure_equals("relative path", p.relative_path().string(), "Windows/System"); 
		ensure_equals("parent path", p.parent_path().string(), "C:/Windows"); 
		ensure_equals("file name", p.filename(), "System");


		win32::file::path p2("photo.jpg"); 
		ensure_equals("term", p2.stem(), "photo"); 
		ensure_equals("extension", p2.extension(), ".jpg"); 


		win32::file::path p3("C:\\"); 
		p3 /= "Windows\\System"; 
		ensure_equals("path operator/=", p3.string(), "C:/Windows/System");


		try 
		{ 
			win32::file::path p("C:\\"); 
			win32::file::file_status s = win32::file::status(p); 
			auto size = win32::file::file_size(p);
			std::cout << win32::file::is_directory(s) << std::endl; 
			win32::file::space_info space = win32::file::space(p);
			std::cout << space.capacity << space.free << space.available << std::endl;

			win32::file::path p2("C:\\Console.zip"); 
			std::cout << win32::file::exists(win32::file::status(p2)) << std::endl;

			std::time_t t = win32::file::last_write_time(p2);
			std::cout << t << std::endl;

			win32::file::path p3("C:\\Test\\Test");
			win32::file::create_directories(p3);
			{
				win32::file::rename(p3, win32::file::path("C:\\Test\\Test2"));
				win32::file::create_directory(win32::file::path("C:\\Test\\Test3"));
				win32::file::remove_all(win32::file::path("C:\\Test"));
			}
		} 
		catch(win32::file::filesystem_error &e) 
		{ 
			std::cerr << e.what() << std::endl; 
		} 
	}


	template<>
	template<>
	void object::test<11>()
	{
		set_test_name("file find");

		auto check_cond = [](const win32::filesystem::find_file &find)->bool{ return true; };
		auto op_cond = [](const std::wstring &file){ std::wcout << file << std::endl;};

		win32::filesystem::depth_search(L"F:\\WX_OL\\include\\win32", check_cond, op_cond); 

	}
}

#include "unit_test/tut/tut/tut.hpp"

#include "utility/smart_handle.hpp"




namespace tut
{
	struct smart_handle_test
	{};

	typedef test_group<smart_handle_test> tf;
	typedef tf::object object;
	tf smart_handle_test_group("smart_handle_test");


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("auto file read write");

		{
			utility::auto_file file = ::CreateFile("C://1.txt", GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		
			if( file )
			{
				char buf[] = "test smart handle";
				DWORD written = 0;
				::WriteFile(file, buf, _countof(buf), &written, 0);
			}

			ensure("open file", file.is_valid());
		}

		::DeleteFile("C://1.txt");
		
		
	}

}
#include "unit_test/tut/tut/tut.hpp"

//#include "extend_stl/mutex.hpp"


#pragma message("==============to do mutex=============")
namespace tut
{
	struct mutex_test
	{};

	typedef test_group<mutex_test> tf;
	typedef tf::object object;
	tf mutex_test_group("mutex_test");

	template<>template<>
	void object::test<1>()
	{
		set_test_name("");
		
		//stdex::mutex mutex;
		//stdex::unique_lock<stdex::mutex> lock(mutex);
	}

}
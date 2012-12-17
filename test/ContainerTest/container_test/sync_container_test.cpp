#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/container/sync_container.hpp"


namespace tut
{
	struct sync_container
	{};

	typedef test_group<sync_container> tf;
	typedef tf::object object;
	tf sync_container_group("sync_container");


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("sync_sequence_container");

		stdex::container::sync_sequence_container_t<int> vec;
		vec.push_back(10);
		ensure("size", vec.size() == 1);

		vec.clear();
		ensure("empty", vec.empty());

		for(size_t i = 0; i != 10; ++i)
			vec.push_back(i);

		ensure("top", 0 == vec.top());
		ensure("operator []", 4 == vec[4]);


		vec.op_if([](int val)->bool{ return val == 8;},
			[](int val){std::cout << val << std::endl;});

		vec.for_each([](int val){ std::cout << val << std::endl;});

		vec.erase([](int val){ return val == 5;});
		ensure("size", vec.size() == 9);
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("sync_assoc_container");

		stdex::container::sync_assoc_container_t<int, std::string> vec;
		vec.insert(10, "test1");
		ensure("size", vec.size() == 1);

		vec.clear();
	ensure("empty", vec.empty());

		for(size_t i = 0; i != 10; ++i)
		{
			std::ostringstream os;
			os << i;
			vec.insert(i, os.str());
		}

		vec.op_if(8, [](const std::pair<int, std::string> &val)->bool{ return val.first == 8;});

		vec.for_each([](const std::pair<int, std::string> &val){ std::cout << val.second << std::endl;});

		ensure("exsit", vec.exsit(5));
		vec.erase(5);
		ensure("size", vec.size() == 9);
	}

}
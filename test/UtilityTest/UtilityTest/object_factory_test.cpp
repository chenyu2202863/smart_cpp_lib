
#include "unit_test/tut/tut/tut.hpp"

#include "utility/object_factory.hpp"




namespace tut
{
	struct object_factory_test
	{};

	typedef test_group<object_factory_test> tf;
	typedef tf::object object;
	tf object_factory_test_group("object_factory_test");


	

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("object_factory_test");

		struct test_type
		{
			int val_;

			test_type()
				: val_(0)
			{}

			test_type(int val)
				: val_(val)
			{}

			operator int() const
			{
				return val_;
			}
		};

		typedef std::shared_ptr<test_type> test_type_ptr;
		typedef utility::object_factory_t<test_type> type_factory;
		
		test_type_ptr val1(utility::object_allocate<test_type>(), utility::object_deallocate<test_type>); 
		test_type_ptr val2(utility::object_allocate<test_type>(1), utility::object_deallocate<test_type>); 

		ensure("val1", *val1 == 0);
		ensure("val2", *val2 == 1);
	}


	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("operator new");

		struct test_type
			: public utility::new_delete_base_t<test_type>
		{
			int val_;

			test_type()
				: val_(0)
			{}

			test_type(int val)
				: val_(val)
			{}

			operator int() const
			{
				return val_;
			}
		};

		std::auto_ptr<test_type> val1(new test_type);
	}
}
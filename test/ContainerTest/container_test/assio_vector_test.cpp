#include "unit_test/tut/tut/tut.hpp"

#include "extend_stl/container/assoc_vector.hpp"


namespace tut
{
	struct assoc_vector
	{};

	typedef test_group<assoc_vector> tf;
	typedef tf::object object;
	tf assoc_vector_group("assoc_vector");


	typedef std::map< std::string, unsigned int > StudentGradeMap;
	typedef StudentGradeMap::iterator StudentGradeMapIter;
	typedef StudentGradeMap::const_iterator StudentGradeMapCIter;

	typedef container::assoc_vector_t< std::string, unsigned int > StudentGrades;
	typedef StudentGrades::iterator StudentGradeIter;
	typedef StudentGrades::const_iterator StudentGradeCIter;

	typedef std::pair<std::string, unsigned int > GradeInfo;


	GradeInfo oneStudent = ::std::make_pair( "Anne", 100 );

	GradeInfo noDuplicates[] =
	{
		::std::make_pair( "Anne", 100 ),
		::std::make_pair( "Bill",  88 ),
		::std::make_pair( "Clay",  91 ),
		::std::make_pair( "Dina",  62 ),
		::std::make_pair( "Evan",  77 ),
		::std::make_pair( "Fran",  84 ),
		::std::make_pair( "Greg",  95 )
	};


	GradeInfo hasDuplicates[] =
	{
		std::make_pair( "Anne", 100 ),
		std::make_pair( "Anne",  73 ),
		std::make_pair( "Bill",  88 ),
		std::make_pair( "Clay",  91 ),
		std::make_pair( "Dina",  62 ),
		std::make_pair( "Evan",  77 ),
		std::make_pair( "Fran",  74 ),
		std::make_pair( "Fran",  84 ),
		std::make_pair( "Greg",  95 )
	};


	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("assoc_vector");

		StudentGrades grades;
		const StudentGrades & cGrades = grades;
		ensure("empty", grades.empty() );
		ensure("size", grades.size() == 0 );
		ensure("empty", cGrades.empty() );
		ensure("size", cGrades.size() == 0 );

		StudentGradeIter it1( grades.begin() );
		ensure("end", it1 == grades.end() );

		const StudentGradeIter it2( grades.begin() );
		ensure("end", it2 == grades.end() );
		ensure("iterator", it2 == it1 );
		ensure("iterator", it1 == it2 );

		StudentGradeCIter cit1( grades.begin() );
		ensure("iterator",  cit1 == grades.end() );
		ensure("iterator",  cit1 == it1 );
		ensure("iterator",  cit1 == it2 );
		ensure("iterator",  it1 == cit1 );
		ensure("iterator",  it2 == cit1 );

		const StudentGradeCIter cit2( grades.begin() );
		ensure("iterator",  cit2 == grades.end() );
		ensure("iterator",  cit1 == cit2 );
		ensure("iterator",  cit2 == cit1 );
		ensure("iterator",  cit2 == it1 );
		ensure("iterator",  cit2 == it2 );
		ensure("iterator",  it1 == cit2 );
		ensure("iterator",  it2 == cit2 );

		StudentGradeCIter cit3( cGrades.begin() );
		ensure("iterator", cit3 == cGrades.end() );
		ensure("iterator", cit3 == it1 );
		ensure("iterator", cit3 == it2 );
		ensure("iterator", it1 == cit3 );
		ensure("iterator", it2 == cit3 );

		const StudentGradeCIter cit4( cGrades.begin() );
		ensure("iterator", cit4 == cGrades.end() );
		ensure("iterator", cit1 == cit4 );
		ensure("iterator", cit4 == cit1 );
		ensure("iterator", cit4 == it1 );
		ensure("iterator", cit4 == it2 );
		ensure("iterator", it1 == cit4 );
		ensure("iterator", it2 == cit4 );

	}

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("assoc_vector");

		static const unsigned int  noDuplicateCount = ( sizeof(noDuplicates)  / sizeof(noDuplicates[0])  );
		static const unsigned int hasDuplicateCount = ( sizeof(hasDuplicates) / sizeof(hasDuplicates[0]) );

		{
			// This test demonstrates the iterator constructor does not allow any duplicate elements.
			StudentGrades grades1(  noDuplicates,  noDuplicates +  noDuplicateCount );
			StudentGrades grades2( hasDuplicates, hasDuplicates + hasDuplicateCount );
			ensure("size",  grades1.size() != 0 );
			ensure("size",  grades2.size() != 0 );
			ensure("size",  grades1.size() == noDuplicateCount );
			ensure("size",  grades2.size() == noDuplicateCount );
			ensure("size",  grades1.size() == grades2.size() );
		}

		{
			// This test demonstrates copy construction.
			StudentGrades grades1( noDuplicates, noDuplicates + noDuplicateCount );
			const StudentGrades grades2( grades1 );
			ensure("size",  grades1.size() != 0 );
			ensure("size",  grades2.size() != 0 );
			ensure("size",  grades1.size() == noDuplicateCount );
			ensure("size",  grades2.size() == noDuplicateCount );
			ensure("size",  grades1.size() == grades2.size() );
			ensure("size",  grades1 == grades2 );

			StudentGrades grades3;
			grades3 = grades1;
			ensure("size",  grades3.size() != 0 );
			ensure("size",  grades3.size() == noDuplicateCount );
			ensure("size",  grades3.size() == grades1.size() );
			ensure("size",  grades1 == grades3 );
		}
	}


	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("assoc_vector find");

		static const unsigned int  noDuplicateCount = sizeof(noDuplicates)  / sizeof(noDuplicates[0]); 

		StudentGrades grades(  noDuplicates,  noDuplicates +  noDuplicateCount );

		ensure("iterator", grades.find("Anne") != grades.end());

		StudentGrades::const_iterator iter = grades.find("Evan");
		ensure("iterator", iter != grades.end());

	}


	template<>
	template<>
	void object::test<5>()
	{
		typedef container::assoc_vector_t<std::string, std::string> Persons;

		Persons persons;
		persons.insert(std::make_pair("1231", "25"));
		persons.insert(std::make_pair("12312", "24"));
		persons.insert(std::make_pair("12313", "2313"));
		persons.insert(std::make_pair("h22", "24"));
		persons.insert(std::make_pair("123", "123"));

		Persons::iterator beg = persons.lower_bound("chenyu");
		Persons::iterator end = persons.upper_bound("heyao");

		persons.erase(beg, end);
	}
}
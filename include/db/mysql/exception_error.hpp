#ifndef __PRO_SQL_EXCEPTION_HPP
#define __PRO_SQL_EXCEPTION_HPP

#include <stdexcept>

#include "config.hpp"


namespace db { namespace mysql {


	class session_t;
	class statement_t;


	class exception_t
		: public std::runtime_error
	{
	public:
		exception_t(session_t &);
	};


	class stmt_exception_t
		: public std::runtime_error
	{
	public:
		stmt_exception_t(statement_t &);
	};

}}

#endif 
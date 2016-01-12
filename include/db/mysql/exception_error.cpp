#include "exception_error.hpp"

#include <WinSock2.h>
#include <mysql.h>

#include "session.hpp"
#include "statement.hpp"

namespace db { namespace mysql {


	exception_t::exception_t(session_t &session)
		: std::runtime_error("error code [" + 
			std::to_string(::mysql_errno(session.native_handle())) + 
			"], " +
			::mysql_error(session.native_handle()))
	{}

	stmt_exception_t::stmt_exception_t(statement_t &statment)
		: std::runtime_error("error code [" +
			std::to_string(::mysql_stmt_errno(statment.native_handle())) +
			"], " +
			::mysql_stmt_error(statment.native_handle()))
	{}

}}
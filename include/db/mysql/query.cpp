#include "query.hpp"

#include <WinSock2.h>
#include <algorithm>
#include <stdexcept>

#include <mysql.h>

#include "exception_error.hpp"
#include "statement.hpp"
#include "session.hpp"

namespace db { namespace mysql {



	query_t::query_t(const std::string &sql)
	{
		sql_ << sql;
	}

	query_t::~query_t()
	{
	}

	std::pair<const char *, std::uint32_t> query_t::sql() const
	{
		return sql_.str();
	}
}}
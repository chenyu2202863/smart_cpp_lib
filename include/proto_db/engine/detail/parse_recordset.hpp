#ifndef __DB_PROTOBUF_PARSE_RECORDSET_HPP
#define __DB_PROTOBUF_PARSE_RECORDSET_HPP

#include <string>

#include "db.hpp"
#include "../config.hpp"


namespace async { namespace proto { namespace detail {

	struct parse_rds
	{
		msg_ptr parse(const std::string &table_name, const recordset_ptr &rds);
	};

}}}

#endif

#ifndef __DB_PROTOBUF_BUILD_SQL_HPP
#define __DB_PROTOBUF_BUILD_SQL_HPP

#include <string>
#include <vector>

#include "../config.hpp"

class _variant_t;

namespace async { namespace proto { namespace detail {
	 
	class command_t;
	typedef std::shared_ptr<command_t> command_ptr;

	class parameters_t;
	typedef std::shared_ptr<parameters_t> parameters_ptr;

	struct build_sql
	{
		std::string build_select(const std::string &table_name, const std::string &columns, const std::string &where);
		std::string build_insert(const gpb::Message &msg);
		std::string build_update(const gpb::Message &msg, const gpb::Message &where);
		std::string build_delete(const gpb::Message &where);

		bool build_binder(const command_ptr &, const gpb::Message &);
		bool build_binder(const command_ptr &, const std::vector<_variant_t> &);
	};

}}}

#endif
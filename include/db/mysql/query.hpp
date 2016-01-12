#ifndef __PRO_SQL_QUERY_HPP
#define __PRO_SQL_QUERY_HPP

#include <sstream>
#include <tuple>

#include "../../utility/stack_stream.hpp"

#include "config.hpp"
#include "into.hpp"
#include "use.hpp"

namespace db { namespace mysql {

	class session_t;
	class statement_t;


	class query_t
	{
		utility::stack_stream_t sql_;

	public:
		query_t() = default;
		explicit query_t(const std::string &sql);
		~query_t();


		query_t(const query_t &) = delete;
		query_t& operator=(const query_t &) = delete;


		std::pair<const char *, std::uint32_t> sql() const;

		template < typename T >
		query_t &operator<<(const T &t)
		{
			sql_ << t;
			return *this;
		}

		query_t &operator<<(const std::pair<const char *, std::uint32_t> &t)
		{
			sql_ << t;
			return *this;
		}
	};


} }

#endif
#ifndef __PRO_SQL_SESSION_HPP
#define __PRO_SQL_SESSION_HPP


#include "config.hpp"
#include "query.hpp"


typedef struct st_mysql MYSQL;

namespace db { namespace mysql {


	class session_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		session_t();

		explicit session_t(const std::string &host,
			std::uint16_t port,
			const std::string &db_name,
			const std::string &user_name,
			const std::string &password,
			std::uint32_t client_flag = 0);

		~session_t();

		session_t(const session_t &) = delete;
		session_t &operator=(const session_t &) = delete;

		void open(const std::string &host,
			std::uint16_t port,
			const std::string &db_name,
			const std::string &user_name,
			const std::string &password,
			std::uint32_t client_flag = 0);
		void close();
		bool is_open() const;

		MYSQL *native_handle() const;

		void clear();

		std::uint64_t last_insert_rowid() const;

		// Execute SQL query immediately.
		// It might be useful for resultless statements like INSERT, UPDATE etc.
		// T is any output-stream-shiftable type.

		template< typename T >
		void operator<<(const T &t)
		{
			once_query_t q(*this);
			q << t;
		}
	};


}}

#endif 

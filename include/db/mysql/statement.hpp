#ifndef __PRO_SQL_STATEMENT_HPP
#define __PRO_SQL_STATEMENT_HPP

#include <vector>
#include <mysql.h>

#include "config.hpp"
#include "query.hpp"
#include "traits.hpp"



namespace db { namespace mysql {

	class session_t;


	struct result_buffer_t
	{
		char buffer_[255];
		unsigned long length_;
		enum_field_types type_;

		result_buffer_t(enum_field_types type, MYSQL_BIND &param)
		{
			std::memset(this, 0, sizeof(*this));
			type_ = type;

			param.buffer_type		= type;
			param.buffer			= buffer_;
			param.buffer_length		= 255;
			param.length			= &length_;
		}
	};

	struct bind_buffer_t
	{
		char buffer_[255];
		
		template < typename T >
		bind_buffer_t(const T &val, MYSQL_BIND &param)
		{
			std::memcpy(buffer_, buffer_address_t<T>::address(&val), buffer_size_t<T>::size(&val));

			param.buffer_type	= cpp_type_to_sql_type_t<T>::value;
			param.buffer_length	= buffer_size_t<T>::size(&val);
			param.buffer		= buffer_;
			param.is_unsigned	= std::is_unsigned<T>::value;

			assert(param.buffer_length < 255);
		}
	};

	class statement_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;

		std::vector<MYSQL_BIND> result_params_;
		std::vector<result_buffer_t> result_buffers_;
		std::vector<std::string> fields_;

	public:
		explicit statement_t(session_t &);
		~statement_t();

		statement_t(const statement_t &) = delete;
		statement_t& operator=(const statement_t &) = delete;

	public:
		MYSQL_STMT *native_handle() const;

		void execute();
		void prepare(const std::pair<const char *, std::uint32_t> &);

		const std::vector<std::string> & prepare_fetch();

		std::pair<const std::vector<result_buffer_t> &, bool> fetch() const;

		void clear();

		std::uint64_t result_count() const;
		std::uint64_t last_insert_rowid() const;

		void bind_params(const MYSQL_BIND *, std::uint32_t);

	private:
		enum class fetch_result_type
		{
			ok, failed, resize
		};
		std::pair<fetch_result_type, MYSQL_ROW_OFFSET> _fetch_impl(details::use_param_t *, std::uint32_t);
		void _offset_impl(MYSQL_ROW_OFFSET);
	};
}}


#endif
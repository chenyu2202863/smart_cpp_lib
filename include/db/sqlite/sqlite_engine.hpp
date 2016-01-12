#ifndef __SQLITE_ENGINE_HPP
#define __SQLITE_ENGINE_HPP

#include <memory>
#include <cstdint>
#include <string>
#include <functional>
#include <cassert>
#include <vector>

#include <extend_stl/variant.hpp>


struct sqlite3;
struct sqlite3_stmt;

namespace db { namespace sqlite {

	class session_t
	{
		sqlite3 *db_ = nullptr;

	public:
		session_t();
		~session_t();

		operator sqlite3 *()
		{
			return db_;
		}

		operator const sqlite3 *() const
		{
			return db_;
		}

		bool is_open() const
		{
			return db_ != nullptr;
		}

		void open(const std::string &path);

		void close();

		std::uint64_t last_insert_rowid() const;

		void check_error(int code) const;
		void check_last_error() const;
	};


	class statement_t
	{
		session_t &s_;
		sqlite3_stmt *impl_ = nullptr;

	public:
		using param_t = stdex::variant_t<std::int64_t, double, std::string>;

		enum class col_type : char
		{
			COL_INT = 1, COL_REAL = 2, COL_TEXT = 3
		};
		using params_t = std::vector<std::pair<col_type, param_t>>;
		params_t params_;

	public:
		explicit statement_t(session_t& s);
		~statement_t();

		bool execute();
		void prepare(const std::pair<const char *, std::uint32_t> &sql);
		void finalize();
		bool is_prepared() const;

		std::uint64_t last_insert_rowid() const;

		const params_t &params() const
		{ return params_; }

		void use_value(int pos, std::int32_t value);
		void use_value(int pos, long long value);
		void use_value(int pos, double value);
		void use_value(int pos, const std::pair<const char *, std::uint32_t> &);

		void use_value(int pos, const std::string &);
	};
}}

namespace baimo { namespace db {

	class engine_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;
		
		using error_handler_t = std::function<void(const std::string &)>;
		error_handler_t error_handler_;

	public:
		engine_t(const error_handler_t &);
		~engine_t();

		engine_t(const engine_t &) = delete;
		engine_t &operator=(const engine_t &) = delete;

		bool start(const std::string &path);
		bool stop();

		::db::sqlite::statement_t *prepare(const std::pair<const char *, std::uint32_t> &);
		
		template < typename T >
		void bind(::db::sqlite::statement_t *, int pos, const T &);

		template < typename HandlerT >
		void execute(::db::sqlite::statement_t *, HandlerT &&);

		template < typename HandlerT >
		void select(::db::sqlite::statement_t *, HandlerT &&);
	};

	template < typename T >
	void engine_t::bind(::db::sqlite::statement_t *st, int pos, const T &val)
	{
		assert(st != nullptr);

		st->use_value(pos, val);
	}

	template < typename HandlerT >
	void engine_t::execute(::db::sqlite::statement_t *st, HandlerT &&handler)
	{
		assert(st != nullptr);

		try
		{
			st->execute();
			handler(true, st->last_insert_rowid());
		}
		catch( const std::exception &e )
		{
			error_handler_(e.what());
			handler(false, 0);
		}
	}

	template < typename HandlerT >
	void engine_t::select(::db::sqlite::statement_t *st, HandlerT &&handler)
	{
		assert(st != nullptr);

		try
		{
			while(st->execute())
				handler(true, st->params());
		}
		catch( const std::exception &e )
		{
			error_handler_(e.what());
			handler(false, st->params());
		}
	}
}}


#endif
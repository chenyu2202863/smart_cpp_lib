#include "engine.hpp"

#include <cassert>
#include <list>

#include "../sqlite/sqlite3.h"


namespace db { namespace sqlite {

	std::string last_error_msg(sqlite3* impl)
	{
		assert(impl != nullptr);
		std::string result = ::sqlite3_errmsg(impl);

		return result;
	}

	session_t::session_t()
	{}

	session_t::~session_t()
	{
		if( db_ != nullptr )
			::sqlite3_close(db_);
	}


	void session_t::open(const std::string &path)
	{
		close();

		auto ret = ::sqlite3_open_v2(path.c_str(), &db_, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, 0);
		check_error(ret);
	}

	void session_t::close()
	{
		if( is_open() )
		{
			::sqlite3_close(db_);
			db_ = nullptr;
		}
	}

	std::uint64_t session_t::last_insert_rowid() const
	{
		return is_open() ? ::sqlite3_last_insert_rowid(db_) : 0;
	}

	void session_t::check_error(int code) const
	{
		if( code != SQLITE_OK && code != SQLITE_ROW && code != SQLITE_DONE )
		{
			throw std::runtime_error(last_error_msg(db_));
		}
	}

	void session_t::check_last_error() const
	{
		check_error(::sqlite3_errcode(db_));
	}


	statement_t::statement_t(session_t& s)
		: s_(s)
	{}

	statement_t::~statement_t()
	{

	}

	bool statement_t::execute()
	{
		try
		{
			auto ret = ::sqlite3_step(impl_);
			switch( ret )
			{
			case SQLITE_ROW:
				{
					for( auto i = 0; i != params_.size(); ++i )
					{
						auto type = static_cast<col_type>(::sqlite3_column_type(impl_, i));
						s_.check_last_error();

						params_[i].first = type;

						switch( type )
						{
						case col_type::COL_INT:
							params_[i].second = ::sqlite3_column_int64(impl_, i);
						break;
						case col_type::COL_REAL:
							params_[i].second = ::sqlite3_column_double(impl_, i);
						break;
						case col_type::COL_TEXT:
							params_[i].second = reinterpret_cast<const char*>(::sqlite3_column_text(impl_, i));
						break;
						default:
						throw std::runtime_error("not support this type");
						break;
						}
						
						s_.check_last_error();
					}
				}
				return true;

			case SQLITE_DONE:
				s_.check_error(::sqlite3_reset(impl_));
				return false;
			
			default:
				s_.check_error(ret);
				return false;
			}
		}
		catch( const std::exception & )
		{
			finalize();
			throw;
		}
	}

	void statement_t::prepare(const std::pair<const char *, std::uint32_t> &sql)
	{
		try
		{
			const char *tail = nullptr;
			s_.check_error(::sqlite3_prepare_v2(s_, sql.first, sql.second, &impl_, &tail));

			auto col_cnt = sqlite3_column_count(impl_);
			s_.check_last_error();

			params_.resize(col_cnt);

			if( tail && *tail )
				throw std::runtime_error(tail);
		}
		catch( const std::exception & )
		{
			finalize();
			throw;
		}
	}

	void statement_t::finalize()
	{
		auto ret = ::sqlite3_finalize(impl_);
		s_.check_error(ret);
	}

	bool statement_t::is_prepared() const // throw()
	{
		return impl_ != 0;
	}

	std::uint64_t statement_t::last_insert_rowid() const
	{
		return s_.last_insert_rowid();
	}

	void statement_t::use_value(int pos, int value)
	{
		s_.check_error(::sqlite3_bind_int(impl_, pos, value));
	}

	void statement_t::use_value(int pos, double value)
	{
		s_.check_error(::sqlite3_bind_double(impl_, pos, value));
	}

	void statement_t::use_value(int pos, long long value)
	{
		s_.check_error(::sqlite3_bind_int64(impl_, pos, value));
	}

	void statement_t::use_value(int pos, const std::pair<const char *, std::uint32_t> &value)
	{
		s_.check_error(::sqlite3_bind_text(impl_, pos, value.first, value.second, SQLITE_STATIC));
	}

	void statement_t::use_value(int pos, const std::string &value)
	{
		s_.check_error(::sqlite3_bind_text(impl_, pos, value.empty() ? 0 : value.c_str(), value.size(), SQLITE_STATIC));
	}
	
}
}


namespace baimo { namespace db {


	struct engine_t::impl
	{
		::db::sqlite::session_t session_;
		
		std::list<std::shared_ptr<::db::sqlite::statement_t>> statements_;
	};

	engine_t::engine_t(const error_handler_t &error_handler)
		: impl_(std::make_unique<impl>())
		, error_handler_(error_handler)
	{}

	engine_t::~engine_t()
	{}

	bool engine_t::start(const std::string &path)
	{
		try
		{
			impl_->session_.open(path);
			return true;
		}
		catch( const std::exception &e )
		{
			error_handler_(e.what());
			return false;
		}
	}

	bool engine_t::stop()
	{
		try
		{
			impl_->session_.close();
			return true;
		}
		catch( const std::exception &e )
		{
			error_handler_(e.what());
			return false;
		}
	}

	::db::sqlite::statement_t *engine_t::prepare(const std::pair<const char *, std::uint32_t> &sql)
	{
		try
		{
			auto statement_val = std::make_shared<::db::sqlite::statement_t>(impl_->session_);
			impl_->statements_.push_back(statement_val);
			statement_val->prepare(sql);

			return statement_val.get();
		}
		catch( const std::exception &e )
		{
			error_handler_(e.what());
			return nullptr;
		}
	}
}}
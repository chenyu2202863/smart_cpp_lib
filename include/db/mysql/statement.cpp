#include "statement.hpp"

#include <WinSock2.h>

#include <algorithm>
#include <numeric>
#include <vector>

#include <mysql.h>

#include "../../extend_stl/allocator/stack_allocator.hpp"
#include "../../utility/scope_exit.hpp"

#include "exception_error.hpp"
#include "session.hpp"
#include "traits.hpp"

namespace db { namespace mysql {

	using stack_storage_t	= stdex::allocator::stack_storage_t<4096>;
	using bind_allocator_t	= stdex::allocator::stack_allocator_t<MYSQL_BIND, 4096>;
	using binder_arrays_t	= std::vector<MYSQL_BIND, bind_allocator_t>;

	struct statement_t::impl
	{
		MYSQL_STMT *stmt_ = nullptr;
		bool is_prepared = false;

		session_t &session_;
		
		impl(session_t &session)
			: session_(session)
		{
			stmt_ = ::mysql_stmt_init(session.native_handle());
			if( stmt_ == nullptr )
				throw exception_t(session_);
		}

		~impl()
		{
			if( stmt_ != nullptr )
			{
				auto ret = ::mysql_stmt_close(stmt_);
				assert(!ret);
			}
		}
	};

	statement_t::statement_t(session_t &s)
		: impl_(std::make_unique<impl>(s))

	{}



	statement_t::~statement_t()
	{}

	MYSQL_STMT *statement_t::native_handle() const
	{
		return impl_->stmt_;
	}

	void statement_t::prepare(const std::pair<const char *, std::uint32_t> &sql)
	{
		assert(!impl_->is_prepared);

		if( impl_->is_prepared )
			throw std::runtime_error("statement was prepared!");

		if( ::mysql_stmt_prepare(impl_->stmt_, sql.first, sql.second) != 0 )
			throw stmt_exception_t(*this);

		impl_->is_prepared = true;
	}

	void statement_t::execute()
	{
		assert(impl_->is_prepared);
		if( !impl_->is_prepared )
			throw std::runtime_error("statement was prepared!");

		if( ::mysql_stmt_execute(impl_->stmt_) != 0 ||
			::mysql_stmt_store_result(impl_->stmt_) != 0 )
			throw stmt_exception_t(*this);
	}


	std::uint64_t statement_t::result_count() const
	{
		return ::mysql_stmt_num_rows(impl_->stmt_);
	}


	void statement_t::clear()
	{
		impl_->session_.clear();
	}


	const std::vector<std::string> &statement_t::prepare_fetch()
	{
		if( fields_.empty() )
		{
			auto meta_result = ::mysql_stmt_result_metadata(impl_->stmt_);
			if( meta_result == nullptr )
				throw stmt_exception_t(*this);

			auto fields	= ::mysql_fetch_fields(meta_result);
			auto result_cnt = ::mysql_num_fields(meta_result);

			fields_.reserve(result_cnt);
			result_buffers_.reserve(result_cnt);
			result_params_.resize(result_cnt);

			for( auto i = 0; i != result_cnt; ++i )
			{
				fields_.emplace_back(fields[i].name);
				result_buffers_.emplace_back(fields[i].type, result_params_[i]);
			}

			::mysql_free_result(meta_result);

			assert(::mysql_stmt_field_count(impl_->stmt_) == result_cnt);

			if( ::mysql_stmt_bind_result(impl_->stmt_, result_params_.data()) != 0 )
				throw stmt_exception_t(*this);
		}

		return fields_;
	}

	std::pair<const std::vector<result_buffer_t> &, bool> statement_t::fetch() const
	{
		auto ret = ::mysql_stmt_fetch(impl_->stmt_);
		assert(ret != MYSQL_DATA_TRUNCATED);

		return std::pair<const std::vector<result_buffer_t> &, bool>(result_buffers_, ret == 0);
	}

	void statement_t::bind_params(const MYSQL_BIND *params, std::uint32_t cnt)
	{
		assert(::mysql_stmt_param_count(impl_->stmt_) == cnt);

		if( ::mysql_stmt_bind_param(impl_->stmt_, const_cast<MYSQL_BIND *>(params)) != 0 )
			throw stmt_exception_t(*this);
	}


	std::pair<statement_t::fetch_result_type, MYSQL_ROW_OFFSET> statement_t::_fetch_impl(details::use_param_t *param, std::uint32_t cnt)
	{
		auto offset = ::mysql_stmt_row_tell(impl_->stmt_);
		auto error = ::mysql_stmt_fetch(impl_->stmt_);
		if( error == MYSQL_DATA_TRUNCATED )
			return { fetch_result_type::resize, offset };
		else
			return { error == 0 ? fetch_result_type::ok : fetch_result_type::failed, nullptr };
	}

	void statement_t::_offset_impl(MYSQL_ROW_OFFSET offset)
	{
		auto ret = ::mysql_stmt_row_seek(impl_->stmt_, offset);
	}

	std::uint64_t statement_t::last_insert_rowid() const
	{
		return ::mysql_stmt_insert_id(impl_->stmt_);
	}

}
}
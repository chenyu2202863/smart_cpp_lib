#include "session.hpp"
#include "exception_error.hpp"

#include <WinSock2.h>
#include <mysql.h>

#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "Advapi32")  
#pragma comment(lib, "User32")


namespace db { namespace mysql {


	struct session_t::impl
	{
		MYSQL handle_;
		MYSQL *conn_ = nullptr;

		impl()
		{
			std::memset(&handle_, 0, sizeof(handle_));
		}
	};

	session_t::session_t()
		: impl_(std::make_unique<impl>())
	{}

	session_t::session_t(const std::string &host,
		std::uint16_t port,
		const std::string &db_name,
		const std::string &user_name,
		const std::string &password,
		std::uint32_t client_flag /* = 0 */)
		: impl_(std::make_unique<impl>())
	{
		open(host, port, db_name, user_name, password, client_flag);
	}

	session_t::~session_t()
	{
		try
		{
			close();
		}
		catch( ... )
		{
		}
	}

	void session_t::open(const std::string &host,
		std::uint16_t port,
		const std::string &db_name,
		const std::string &user_name,
		const std::string &password,
		std::uint32_t client_flag /* = 0 */)
	{
		assert(!host.empty());
		assert(!user_name.empty());
		assert(!password.empty());

		close();

		impl_->conn_ = ::mysql_init(&impl_->handle_);
		if( ::mysql_real_connect(impl_->conn_,
			host.c_str(),
			user_name.c_str(),
			password.c_str(),
			db_name.c_str(),
			port,
			nullptr,
			CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS) == nullptr )
		{
			throw exception_t(*this);
		}

		::mysql_options(impl_->conn_, MYSQL_SET_CHARSET_NAME, "utf8");
		//::mysql_query(impl_->conn_, "SET NAMES utf8");
	}

	void session_t::close()
	{
		if( is_open() )
		{
			::mysql_close(impl_->conn_);

			std::memset(&impl_->handle_, 0, sizeof(impl_->handle_));
			impl_->conn_ = nullptr;
		}
	}

	bool session_t::is_open() const
	{
		return impl_->conn_ != nullptr;
	}

	MYSQL *session_t::native_handle() const
	{
		assert(is_open());
		return impl_->conn_;
	}

	void session_t::clear()
	{
		assert(is_open());

		while( ::mysql_next_result(impl_->conn_) == 0 )
		{
			auto result = ::mysql_store_result(impl_->conn_);
			if( result != nullptr )
			{
				::mysql_free_result(result);
			}
		}
	}

	std::uint64_t session_t::last_insert_rowid() const
	{
		return is_open() ? ::mysql_insert_id(impl_->conn_) : 0;
	}


}}
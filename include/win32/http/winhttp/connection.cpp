#include "connection.hpp"

#include "session.hpp"
#include "url.hpp"



namespace http
{
	connection_t::connection_t(const session_t &session_val, const url_t &url_path)
		: handle_t(::WinHttpConnect(session_val, url_path.host(), url_path.port(), 0))
		, url_(url_path)
	{
		if( !is_valid() )
			throw http_error_t();
	}
}
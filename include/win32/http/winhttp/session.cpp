#include "session.hpp"
#include <cassert>


namespace http
{
	session_t::session_t(const std::wstring &agent)
		: handle_t(::WinHttpOpen(agent.c_str(),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		WINHTTP_FLAG_ASYNC))
	{
		if( !is_valid() )
			throw http_error_t();
	}

	void session_t::set_timeout(
		std::uint32_t resolve, 
		std::uint32_t connect,
		std::uint32_t send, 
		std::uint32_t recv)
	{
		assert(is_valid());

		BOOL suc = ::WinHttpSetTimeouts(*this, (int)resolve, (int)connect, 
			(int)send, (int)recv);
		assert(suc);
	}
}
#include "handle.hpp"

#include <cassert>


namespace http
{

	handle_t::handle_t() 
		: handle_(0)
	{}

	handle_t::~handle_t()
	{
		if( 0 != handle_ )
		{   
			BOOL suc = ::WinHttpCloseHandle(handle_);
			assert(suc);
			handle_ = 0;
		}
	}


	handle_t::handle_t(HINTERNET handle)
		: handle_(handle)
	{}


	HRESULT handle_t::query_option(DWORD option, void* value, DWORD& length) const
	{
		if( !::WinHttpQueryOption(handle_, option, value, &length) )
		{
			return HRESULT_FROM_WIN32(::GetLastError());
		}

		return S_OK;
	}


}
#include "url.hpp"


namespace http
{
	url_t::url_t(const std::wstring &url_path)
		: url_(url_path)
	{
		std::uninitialized_fill_n(host_, MAX_LENGTH, 0);
		std::uninitialized_fill_n(path_, MAX_LENGTH, 0);

		URL_COMPONENTS urlComp		= {0};
		urlComp.dwStructSize		= sizeof(urlComp);
		urlComp.lpszHostName		= host_;
		urlComp.dwHostNameLength	= MAX_LENGTH;
		urlComp.lpszUrlPath			= path_;
		urlComp.dwUrlPathLength		= MAX_LENGTH;
		if( !::WinHttpCrackUrl(url_path.c_str(), (DWORD)url_path.length(), 0, &urlComp) )
			throw http_error_t();

		secure_ = urlComp.nScheme == INTERNET_SCHEME_HTTPS;
		port_ = urlComp.nPort;
	}
}
#include "win_http.hpp"

#include "url.hpp"
#include "handle.hpp"
#include "session.hpp"
#include "connection.hpp"
#include "request.hpp"

namespace http {

	struct win_http_t::impl
		: std::enable_shared_from_this<impl>
	{
		session_t session_;  
		url_t url_;  
		connection_t connection_;  

		type_t type_;
		error_handler_t error_handler_;

		impl(const std::wstring &url, type_t type, const error_handler_t &error_handler)
			: session_(L"User-Agent")
			, url_(url)
			, connection_(session_, url_)
			, type_(type == get ? get : post)
			, error_handler_(error_handler)
		{}

		~impl()
		{

		}

		
	};

	win_http_t::win_http_t(const std::wstring &url, type_t type, const error_handler_t &error_handler)
		: impl_(new impl(url, type, error_handler))
	{}

	win_http_t::~win_http_t()
	{

	}

	std::vector<char> win_http_t::send(const std::map<std::wstring, std::wstring> &header, const std::string &params)
	{
		sync_request_t request(impl_->connection_, impl_->type_);
		for(std::map<std::wstring, std::wstring>::const_iterator iter = header.begin(); iter != header.end(); ++iter )
		{
			if( !request.add_header(iter->first, iter->second) )
				throw http_error_t();
		}

		return request.send_request(L"", std::make_pair(params.data(), params.size()));
	}


	void win_http_t::timeout(std::uint32_t seconds)
	{
		impl_->session_.set_timeout(seconds * 1000, seconds * 1000, seconds * 1000, seconds * 1000);
	}
}
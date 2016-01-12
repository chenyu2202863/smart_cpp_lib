#ifndef __ASYNC_WIN_HTTP_HPP
#define __ASYNC_WIN_HTTP_HPP

#include "common.hpp"

namespace http {

	struct win_http_t
	{
		struct impl;
		std::auto_ptr<impl> impl_;

		typedef std::function<void(const std::string &)> error_handler_t;

		win_http_t(const std::wstring &, type_t, const error_handler_t&);
		~win_http_t();

		std::vector<char> send(const std::map<std::wstring, std::wstring> &, const std::string &);
		void timeout(std::uint32_t);
	};
}

#endif
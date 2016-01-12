#ifndef __WIN_HTTP_CONNECTION_HPP
#define __WIN_HTTP_CONNECTION_HPP

#include "handle.hpp"
#include "url.hpp"

namespace http
{
	class session_t;

	class connection_t 
		: public handle_t
	{
		url_t url_;

	public:
		connection_t(const session_t &session_val, const url_t &url_path);

	private:
		connection_t(const connection_t &);
		connection_t &operator=(const connection_t &);

	public:
		const url_t &url() const
		{ 
			return url_; 
		}
	};
}




#endif
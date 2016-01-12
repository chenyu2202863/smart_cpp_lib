#ifndef __WIN_HTTP_SESSION_HPP
#define __WIN_HTTP_SESSION_HPP

#include "handle.hpp"


namespace http
{
	class session_t 
		: public handle_t
	{
	public:
		session_t(const std::wstring &agent);
		
	private:
		session_t(const session_t &);
		session_t &operator=(const session_t &);

	public:
		// ms
		void set_timeout(
			std::uint32_t resolve, 
			std::uint32_t connect,
			std::uint32_t send, 
			std::uint32_t recv);
	};
}



#endif
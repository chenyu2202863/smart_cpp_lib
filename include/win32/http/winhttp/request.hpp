#ifndef __WIN_HTTP_REQUEST_HPP
#define __WIN_HTTP_REQUEST_HPP


#include "common.hpp"
#include "handle.hpp"
#include "query_header.hpp"

namespace http
{
	class connection_t;

	class sync_request_t
		: public handle_t
	{
	public:
		sync_request_t(const connection_t &con, type_t);

	private:
		sync_request_t(const sync_request_t &);
		sync_request_t &operator=(const sync_request_t &);

	public:
		bool add_header(const std::wstring &, const std::wstring &);
		std::vector<char> send_request(const std::wstring &headers = L"", const std::pair<const void *, std::uint32_t> &request_data = std::make_pair((const void *)0, 0));
	};


	// --------------
	class async_request_t 
		: public handle_t
	{
	public:
		typedef std::function<void(const async_request_t &, std::uint32_t)>	handle_header_complete_t;
		typedef std::function<void(bool suc)>							handle_response_complete_t;
		typedef std::function<bool(const char *buf, std::uint32_t len)>		handle_read_t;
		typedef std::function<void(const wchar_t *url, std::uint32_t len)>		handle_redirect_t;
		typedef std::function<void(const std::string &)>				handle_error_t;

	public:
		handle_header_complete_t header_complete_;
		handle_response_complete_t response_complete_;
		handle_read_t read_;
		handle_redirect_t redirect_;
		handle_error_t error_;

		std::array<char, 8192> buf_;

	public:
		async_request_t(const connection_t &con, type_t);
		~async_request_t();

	private:
		async_request_t(const async_request_t &);
		async_request_t &operator=(const async_request_t &);

	public:
		void send_request(const std::wstring &headers = L"", const std::pair<const void *, std::uint32_t> &request_data = std::make_pair((const void *)0, 0));
		void register_callback(const handle_header_complete_t &header_complete, const handle_response_complete_t &response_complete, 
			const handle_read_t &read, const handle_redirect_t &redirect, const handle_error_t &error);

	private:
		static void CALLBACK callback(HINTERNET handle, DWORD_PTR context, DWORD code, void* info, DWORD length);
		bool _on_callback(DWORD code, const void* info, DWORD length);
	};
}



#endif
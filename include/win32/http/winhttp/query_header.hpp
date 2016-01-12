#ifndef __WIN_HTTP_QUERY_HEADER_HPP
#define __WIN_HTTP_QUERY_HEADER_HPP


namespace http
{
	namespace query
	{


		template < int LevelT >
		struct header_t
		{
			wchar_t buffer_[4096];
			DWORD buffer_len_;
			std::vector<wchar_t> dynamic_buffer_;

			header_t()
				: buffer_len_(_countof(buffer_))
			{
				std::uninitialized_fill(buffer_, buffer_ + buffer_len_, 0);
			}

			DWORD level() const
			{
				return LevelT;
			}

			const wchar_t *name() const
			{
				return WINHTTP_HEADER_NAME_BY_INDEX;
			}

			void *buffer()
			{
				if( dynamic_buffer_.size() != 0 )
					return &dynamic_buffer_[0];

				return buffer_;
			}

			wchar_t *buffer() const
			{
				if( dynamic_buffer_.size() != 0 )
					return dynamic_buffer_.data();

				return buffer_;
			}

			DWORD *length()
			{
				return &buffer_len_;
			}

			DWORD length() const
			{
				return buffer_len_;
			}

			DWORD *index()
			{
				return WINHTTP_NO_HEADER_INDEX;
			}

			void resize(size_t len)
			{
				dynamic_buffer_.resize(len);
			}
		};



		template < >
		struct header_t<WINHTTP_QUERY_CUSTOM>
		{
			~header_t()
			{
				//static_assert(false, "not support");
			}
		};


		typedef header_t<WINHTTP_QUERY_STATUS_CODE>						status_code;
		typedef header_t<WINHTTP_QUERY_RAW_HEADERS_CRLF>				raw_headers;
		typedef header_t<WINHTTP_QUERY_CONTENT_TYPE>					content_type;
		typedef header_t<WINHTTP_QUERY_CONTENT_LENGTH>					content_length;
		typedef header_t<WINHTTP_QUERY_DATE>							date;
		typedef header_t<WINHTTP_QUERY_EXPIRES>							expires;
		typedef header_t<WINHTTP_QUERY_ACCEPT_CHARSET>					accept_charset;
		typedef header_t<WINHTTP_QUERY_ACCEPT_ENCODING>					accept_encoding;
		typedef header_t<WINHTTP_QUERY_ACCEPT_LANGUAGE>					accept_language;

	}

	template < typename RequestT, int level >
	bool query_http_header(RequestT &request, query::header_t<level> &val)
	{
		BOOL suc = ::WinHttpQueryHeaders(request, val.level(), val.name(), val.buffer(), val.length(), val.index());
		if( !suc && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER )
		{
			val.resize(*(val.length()));
			suc = ::WinHttpQueryHeaders(request, val.level(), val.name(), val.buffer(), val.length(), val.index());
		}

		return suc == TRUE;
	}
}




#endif
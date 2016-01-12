#ifndef __WIN_HTTP_HANDLE_HPP
#define __WIN_HTTP_HANDLE_HPP

#include "common.hpp"
#include "option.hpp"

namespace http
{
	
	class handle_t
	{
	private:
		HINTERNET handle_;

	public:
		handle_t();
		handle_t(HINTERNET handle);

		~handle_t();

	private:
		handle_t(const handle_t &);
		handle_t &operator=(const handle_t &);

	public:
		template < int FlagT, typename OptionT >
		HRESULT set_option(option::option_t<FlagT, OptionT> &op)
		{
			if( !::WinHttpSetOption(handle_, op.flag(), op.option(), op.length()) )
			{
				return HRESULT_FROM_WIN32(::GetLastError());
			}

			return S_OK;
		}

		HRESULT query_option(DWORD option, void* value, DWORD& length) const;

		operator HINTERNET()
		{ return handle_; }

		operator const HINTERNET &() const
		{ return handle_; }

		HINTERNET get()
		{ return handle_; }

		const HINTERNET &get() const
		{ return handle_; }

		bool is_valid() const
		{ return handle_ != 0; }
	};
}




#endif
#ifndef __SYSTEM_SYSTEM_HPP
#define __SYSTEM_SYSTEM_HPP

#include <cstdint>
#include <string>
#include <filesystem>




namespace sys {

	namespace fs = std::tr2::sys;

	namespace details {

		struct ret_helper_t
		{
			std::wstring buffer_;
			explicit ret_helper_t(const wchar_t *buffer);

			operator std::uint8_t() const;
			operator std::uint16_t() const;
			operator std::uint32_t() const;
			operator std::uint64_t() const;

			operator std::string() const;
			operator std::wstring() const;
		};
	}



	std::wstring app_path();

	details::ret_helper_t ini_content(const fs::wpath &, const std::wstring &section, const std::wstring &key);
}


#endif
#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>

namespace stdex
{
	std::string to_string(const std::wstring &s);
	std::wstring to_wstring(const std::string &s);

	std::string to_u8string(const std::u16string& s);
	std::string to_u8string(const std::u32string& s);

	std::u16string to_u16string(const std::string& s);
	std::u16string to_u16string(const std::u32string& s);

	std::u32string to_u32string(const std::string& s);
	std::u32string to_u32string(const std::u16string& s);

}


#endif

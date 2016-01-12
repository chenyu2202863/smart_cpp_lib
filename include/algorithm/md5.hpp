#ifndef __ALGORITHM_MD5_HPP
#define __ALGORITHM_MD5_HPP

#include <tchar.h>
#include <string>


namespace algorithm
{
	namespace md5
	{
		void encode(const void* data, size_t ByteLen, TCHAR *md5, size_t len);

		std::basic_string<TCHAR> encode_string(const char* pString, size_t len);
	}
	
}

#endif
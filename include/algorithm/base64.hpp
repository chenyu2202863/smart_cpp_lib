#ifndef __ALOGRITHM_BASE64_HPP
#define __ALOGRITHM_BASE64_HPP


#include <atlbase.h>
#include <atlenc.h>

#include "../Unicode/string.hpp"


/*
编码后的数据比原始数据略长，为原来的4/3

在电子邮件中，根据RFC 822规定，每76个字符，还需要加上一个回车换行。可以估算编码后数据长度大约为原长的135.1%

如果最后剩下两个输入数据，在编码结果后加1个“=”；如果最后剩下一个输入数据，编码结果后加2个“=”；如果没有剩下任何数据，就什么都不要加

*/

namespace algorithm
{
	namespace base64
	{
		
		// 接口可以扩展

		template < typename CharT >
		inline std::basic_string<CharT> encode(const std::basic_string<CharT> &val)
		{
			std::string sSrcString = unicode::to_a(val);
			int len = ATL::Base64EncodeGetRequiredLength(sSrcString.length());
			
			std::string dest;
			dest.resize(len + 1);
			ATL::Base64Encode(reinterpret_cast<const BYTE *>(sSrcString.c_str()), sSrcString.length(), &dest[0], &len);
			dest.resize(len);

			return unicode::translate_t<CharT>::utf(dest);
		}

		template < typename CharT >
		inline std::basic_string<CharT> decode(const std::basic_string<CharT> &val)
		{
			std::string sSrcString = unicode::to_a(val);
			int len = ATL::Base64DecodeGetRequiredLength(sSrcString.length()) + 1;
			std::string vec;
			vec.resize(len + 1);
			ATL::Base64Decode(sSrcString.c_str(), sSrcString.length(), reinterpret_cast<BYTE*>(&vec[0]), &len);
			vec.resize(len);

			return unicode::translate_t<CharT>::utf(vec);
		}
	}
}





#endif
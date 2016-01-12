#include "Converter.h"

#include <cstdint>

namespace stdex
{

	struct UTF8
	{
		typedef char storage_type;
	};

	struct UTF16 
	{
		typedef char16_t storage_type;
	};

	struct UTF32 
	{
		typedef char32_t storage_type;
	};

	struct CHAR 
	{
		typedef char storage_type;
	};

	struct WCHAR 
	{
		typedef wchar_t storage_type;
	};

	template< typename T, typename F > 
	std::uint32_t storage_size_t();


	template<>
	std::uint32_t storage_size_t<WCHAR, CHAR>() 
	{ return sizeof(char); }

	template<>
	std::uint32_t storage_size_t<CHAR, WCHAR>() 
	{ return sizeof(wchar_t); }


	template<>
	std::uint32_t storage_size_t<UTF8, UTF32>() 
	{ return 4; }

	template<>
	std::uint32_t storage_size_t<UTF8, UTF16>() 
	{ return 3; }

	template<>
	std::uint32_t storage_size_t<UTF16, UTF8>() 
	{ return 1; }

	template<>
	std::uint32_t storage_size_t<UTF16, UTF32>() 
	{ return 2; }

	template<>
	std::uint32_t storage_size_t<UTF32, UTF8>() 
	{ return 1; }

	template<>
	std::uint32_t storage_size_t<UTF32, UTF16>() 
	{ return 1; }


	template< typename T, typename F >
	class converter_t 
	{
		typedef typename F::storage_type from_storage_type;
		typedef typename T::storage_type to_storage_type;

		typedef mbstate_t EncSt;
		typedef std::codecvt<from_storage_type, to_storage_type, EncSt> codecvt_type;

		typedef std::basic_string<from_storage_type> from_string_type;
		typedef std::basic_string<to_storage_type>	 to_string_type;

	public:
		to_string_type operator()(const from_string_type& s)
		{
			static std::locale loc(std::locale::classic(), new codecvt_type);
			static EncSt state;//(F::iconvName(), T::iconvName());
			static const codecvt_type& cvt = std::use_facet<codecvt_type>(loc);


			int len = s.length() * storage_size_t<T, F>();
			to_string_type tmp;
			tmp.resize(len + 1);

			const from_storage_type* enx = nullptr;
			to_storage_type* inx = nullptr;

			typename codecvt_type::result r =
				cvt.out(state, s.c_str(), s.c_str() + s.length(), enx, &tmp[0], &tmp[len], inx);

			if (r != codecvt_type::ok)
				throw std::range_error("Conversion failed");

			return std::move(tmp);//to_string_type(&i[0], inx - &i[0]);
		}
	};



	std::string to_string(const std::wstring &s)
	{
		static converter_t<CHAR, WCHAR> converter;

		return std::move(converter(s));
	}

	std::wstring to_wstring(const std::string &s)
	{
		static converter_t<WCHAR, CHAR> converter;

		return std::move(converter(s));
	}

	std::string to_u8string(const std::u16string& s)
	{
		static converter_t<UTF8, UTF16> converter;

		return std::move(converter(s));
	}

	std::string to_u8string(const std::u32string& s)
	{
		static converter_t<UTF8, UTF32> converter;

		return std::move(converter(s));
	}

	std::u16string to_u16string(const std::string& s)
	{
		static converter_t<UTF16, UTF8> converter;

		return std::move(converter(s));
	}

	std::u16string to_u16string(const std::u32string& s)
	{
		static converter_t<UTF16, UTF32> converter;

		return std::move(converter(s));
	}

	std::u32string to_u32string(const std::string& s)
	{
		static converter_t<UTF32, UTF8> converter;

		return std::move(converter(s));
	}

	std::u32string to_u32string(const std::u16string& s)
	{
		static converter_t<UTF32, UTF16> converter;

		return std::move(converter(s));
	}


}
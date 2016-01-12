#ifndef __DB_PROTOBUF_PARAMETER_TRAITS_HPP
#define __DB_PROTOBUF_PARAMETER_TRAITS_HPP

#include <unordered_map>
#include <functional>
#include <comutil.h>
#include <string>
#include <cstdint>
#include <tuple>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace async { namespace proto { namespace detail {

	enum DataTypeEnum
	{
		adEmpty = 0,
		adTinyInt = 16,
		adSmallInt = 2,
		adInteger = 3,
		adBigInt = 20,
		adUnsignedTinyInt = 17,
		adUnsignedSmallInt = 18,
		adUnsignedInt = 19,
		adUnsignedBigInt = 21,
		adSingle = 4,
		adDouble = 5,
		adCurrency = 6,
		adDecimal = 14,
		adNumeric = 131,
		adBoolean = 11,
		adError = 10,
		adUserDefined = 132,
		adVariant = 12,
		adIDispatch = 9,
		adIUnknown = 13,
		adGUID = 72,
		adDate = 7,
		adDBDate = 133,
		adDBTime = 134,
		adDBTimeStamp = 135,
		adBSTR = 8,
		adChar = 129,
		adVarChar = 200,
		adLongVarChar = 201,
		adWChar = 130,
		adVarWChar = 202,
		adLongVarWChar = 203,
		adBinary = 128,
		adVarBinary = 204,
		adLongVarBinary = 205,
		adChapter = 136,
		adFileTime = 64,
		adPropVariant = 138,
		adVarNumeric = 139,
		adArray = 8192
	};

	typedef std::tuple<_variant_t, DataTypeEnum, std::uint32_t> param_t;

	template < typename T >
	param_t param_traits(const T &val)
	{
		static_assert( false, "not support this type" );
	}

	template < typename T >
	param_t param_traits(const T &val, typename std::enable_if<std::is_enum<T>::value>::type * = nullptr)
	{
		return std::make_tuple(val, adTinyInt, sizeof(val));
	}

	inline param_t param_traits(bool val)
	{
		return std::make_tuple(val, adBoolean, sizeof(val));
	}

	inline param_t param_traits(char val)
	{
		return std::make_tuple(val, adChar, sizeof( val ));
	}

	inline param_t param_traits(unsigned char val)
	{
		return std::make_tuple(val, adUnsignedTinyInt, sizeof( val ));
	}

	inline param_t param_traits(std::int32_t val)
	{
		return std::make_tuple(val, adInteger, sizeof(val));
	}

	inline param_t param_traits(std::uint32_t val)
	{
		return std::make_tuple(val, adUnsignedInt, sizeof( val ));
	}

	inline param_t param_traits(std::int64_t val)
	{
		return std::make_tuple(val, adBigInt, sizeof( val ));
	}

	inline param_t param_traits(std::uint64_t val)
	{
		return std::make_tuple(val, adUnsignedBigInt, sizeof( val ));
	}

	inline param_t param_traits(double val)
	{
		return std::make_tuple(val, adDouble, sizeof( val ));
	}

	inline param_t param_traits(float val)
	{
		return std::make_tuple(val, adDouble, sizeof( val ));
	}


	template < typename CharT, typename TraitsT, typename AllocatorT >
	param_t param_traits(const std::basic_string<CharT, TraitsT, AllocatorT> &val)
	{
		return std::make_tuple(val.c_str(), adVarChar, val.size());
	}

	typedef gpb::FieldDescriptor::CppType cpp_t;

	template < std::uint32_t >
	struct translate_impl_t;

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_INT32>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetInt32(msg, filed_desc));
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_INT64>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetInt64(msg, filed_desc));
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_UINT32>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetUInt32(msg, filed_desc));
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_UINT64>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetUInt64(msg, filed_desc));
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_DOUBLE>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetDouble(msg, filed_desc));
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_BOOL>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetBool(msg, filed_desc));
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_ENUM>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetEnum(msg, filed_desc)->number());
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_FLOAT>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetFloat(msg, filed_desc));
		}
	};

	template <>
	struct translate_impl_t<cpp_t::CPPTYPE_STRING>
	{
		static param_t to(const gpb::Message &msg, const gpb::Reflection *reflection, const gpb::FieldDescriptor *filed_desc)
		{
			return param_traits(reflection->GetString(msg, filed_desc));
		}
	};


	struct param_type_2_db_type_t
	{
		typedef std::function<param_t (const gpb::Message &, const gpb::Reflection *, const gpb::FieldDescriptor *)> translate_t;

		std::unordered_map<cpp_t, translate_t> translate_map_;

		param_type_2_db_type_t()
		{
			translate_map_[cpp_t::CPPTYPE_INT32]	= &translate_impl_t<cpp_t::CPPTYPE_INT32>::to;
			translate_map_[cpp_t::CPPTYPE_INT64]	= &translate_impl_t<cpp_t::CPPTYPE_INT64>::to;
			translate_map_[cpp_t::CPPTYPE_UINT32]	= &translate_impl_t<cpp_t::CPPTYPE_UINT32>::to;
			translate_map_[cpp_t::CPPTYPE_UINT64]	= &translate_impl_t<cpp_t::CPPTYPE_UINT64>::to;
			translate_map_[cpp_t::CPPTYPE_DOUBLE]	= &translate_impl_t<cpp_t::CPPTYPE_DOUBLE>::to;
			translate_map_[cpp_t::CPPTYPE_FLOAT]	= &translate_impl_t<cpp_t::CPPTYPE_FLOAT>::to;
			translate_map_[cpp_t::CPPTYPE_BOOL]		= &translate_impl_t<cpp_t::CPPTYPE_BOOL>::to;
			translate_map_[cpp_t::CPPTYPE_ENUM]		= &translate_impl_t<cpp_t::CPPTYPE_ENUM>::to;
			translate_map_[cpp_t::CPPTYPE_STRING]	= &translate_impl_t<cpp_t::CPPTYPE_STRING>::to;
		}

		const translate_t &operator[](cpp_t type) const
		{
			auto iter = translate_map_.find(type);
			assert(iter != translate_map_.cend());
			if(iter == translate_map_.cend())
				throw std::runtime_error("not find this type");
			else
				return iter->second;
		}
	};

}}}

#endif
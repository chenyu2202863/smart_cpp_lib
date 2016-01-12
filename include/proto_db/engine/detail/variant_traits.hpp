#ifndef __DB_PROTOBUF_VARIANT_TRAITS_HPP
#define __DB_PROTOBUF_VARIANT_TRAITS_HPP

#include <comutil.h>


namespace async { namespace proto { namespace detail {


	inline DataTypeEnum variant_type(const _variant_t &var)
	{
		switch( var.vt )
		{
		case VT_I1:
			return DataTypeEnum::adChar;
		case VT_BOOL:
			return DataTypeEnum::adBoolean;
		case VT_INT:
		case VT_I4:
			return DataTypeEnum::adInteger;
		case VT_R4:
			return DataTypeEnum::adDouble;
		case VT_R8:
			return DataTypeEnum::adDouble;
		case VT_BSTR:
		case VT_LPSTR:
		case VT_LPWSTR:
			return DataTypeEnum::adVarChar;
		default:
			assert(0);
			return DataTypeEnum::adVarChar;
			break;
		}
	}
}}}


#endif
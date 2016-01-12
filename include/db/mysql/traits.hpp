#ifndef __PRO_SQL_TYPE_TRAITS_HPP
#define __PRO_SQL_TYPE_TRAITS_HPP

#include <type_traits>
#include <ctime>
#include <cstdint>
#include <string>

#include <mysql.h>

namespace db { namespace mysql {


	template < typename T >
	struct cpp_type_to_sql_type_t;

	template < >
	struct cpp_type_to_sql_type_t<bool>
	{
		static const enum_field_types value = MYSQL_TYPE_TINY;
	};

	template < >
	struct cpp_type_to_sql_type_t<std::int8_t>
	{
		static const enum_field_types value = MYSQL_TYPE_TINY;
	};

	template < >
	struct cpp_type_to_sql_type_t<std::uint8_t>
	{
		static const enum_field_types value = MYSQL_TYPE_TINY;
	};

	template < >
	struct cpp_type_to_sql_type_t<std::int16_t>
	{
		static const enum_field_types value = MYSQL_TYPE_SHORT;
	};

	template < >
	struct cpp_type_to_sql_type_t<std::uint16_t>
	{
		static const enum_field_types value = MYSQL_TYPE_SHORT;
	};
	
	template < >
	struct cpp_type_to_sql_type_t<std::int32_t>
	{
		static const enum_field_types value = MYSQL_TYPE_LONG;
	}; 

	template < >
	struct cpp_type_to_sql_type_t<std::uint32_t>
	{
		static const enum_field_types value = MYSQL_TYPE_LONG;
	};

	template < >
	struct cpp_type_to_sql_type_t<std::int64_t>
	{
		static const enum_field_types value = MYSQL_TYPE_LONGLONG;
	};

	template < >
	struct cpp_type_to_sql_type_t<std::uint64_t>
	{
		static const enum_field_types value = MYSQL_TYPE_LONGLONG;
	};
	
	template < >
	struct cpp_type_to_sql_type_t<float>
	{
		static const enum_field_types value = MYSQL_TYPE_FLOAT;
	}; 

	template < >
	struct cpp_type_to_sql_type_t<double>
	{
		static const enum_field_types value = MYSQL_TYPE_LONGLONG;
	};

	template < >
	struct cpp_type_to_sql_type_t<long>
	{
		static const enum_field_types value = MYSQL_TYPE_LONG;
	};

	template < >
	struct cpp_type_to_sql_type_t<unsigned long>
	{
		static const enum_field_types value = MYSQL_TYPE_LONGLONG;
	};
	
	template < typename CharT, typename AllocatorT >
	struct cpp_type_to_sql_type_t<std::basic_string<CharT, std::char_traits<CharT>, AllocatorT>>
	{
		static const enum_field_types value = MYSQL_TYPE_VAR_STRING;
	};

	template < >
	struct cpp_type_to_sql_type_t<char *>
	{
		static const enum_field_types value = MYSQL_TYPE_VARCHAR;
	};

	template < >
	struct cpp_type_to_sql_type_t<std::pair<const char *, std::uint32_t>>
	{
		static const enum_field_types value = MYSQL_TYPE_VARCHAR;
	};


	// ------------------
	template < typename T, typename EnableT = void >
	struct buffer_size_t;

	template < typename T >
	struct buffer_size_t<T, typename std::enable_if<std::is_pod<T>::value>::type>
	{
		static std::uint32_t size(const T *)
		{
			return sizeof(T);
		}
	};

	template < >
	struct buffer_size_t<std::string>
	{
		static std::uint32_t size(const std::string *val)
		{
			if( val->empty() )
				const_cast<std::string *>(val)->resize(15);

			return (std::uint32_t)val->size();
		}
	};

	template < >
	struct buffer_size_t<std::pair<const char *, std::uint32_t>>
	{
		static std::uint32_t size(const std::pair<const char *, std::uint32_t> *val)
		{
			return val->second;
		}
	};


	// ------------------

	template < typename T, typename EnableT = void >
	struct buffer_address_t;

	template < typename T >
	struct buffer_address_t<T, typename std::enable_if<std::is_pod<T>::value>::type>
	{
		static void *address(const T *val)
		{
			return (void *) val;
		}
	};

	template < >
	struct buffer_address_t<std::string>
	{
		static void *address(const std::string *val)
		{
			return (void *) val->data();
		}
	};

	template < >
	struct buffer_address_t<std::pair<const char *, std::uint32_t>>
	{
		static void *address(const std::pair<const char *, std::uint32_t> *val)
		{
			return (void *)val->first;
		}
	};
}}

#endif
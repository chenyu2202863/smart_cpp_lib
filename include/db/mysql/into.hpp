#ifndef __PRO_SQL_INTO_HPP
#define __PRO_SQL_INTO_HPP

#include <array>


#include "binders.hpp"

namespace db { namespace mysql {


	namespace details 
	{
		struct into_param_t
		{
			enum_field_types buffer_type_;
			unsigned long buffer_length_ = 0;
			void *buffer_ = nullptr;
			bool is_unsigned_ = false;

			into_param_t(enum_field_types type, unsigned long length, void *buffer, bool is_unsigned)
				: buffer_type_(type)
				, buffer_length_(length)
				, buffer_(buffer)
				, is_unsigned_(is_unsigned)
			{}
		};

		template <>
		struct cast_t<into_param_t>
		{
			template < std::uint32_t Index, std::uint32_t N, typename T >
			static void cast(std::array<into_param_t, N> &buffer, const T *val)
			{
				auto &param = buffer[Index];

				param.buffer_type_	= cpp_type_to_sql_type_t<T>::value;
				param.buffer_length_ = buffer_size_t<T>::size(val);
				param.buffer_		= buffer_address_t<T>::address(val);
				param.is_unsigned_	= std::is_unsigned<T>::value;
			}
		};
	}
	
	


	template < typename ...Args >
	using into_params_t = details::params_t<true, details::into_param_t, Args...>;


	template < typename ...Args >
	into_params_t<Args...> into(const Args &...args)
	{
		return into_params_t<Args...>(args...);
	}

}}

#endif
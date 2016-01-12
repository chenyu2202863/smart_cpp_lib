#ifndef __PRO_SQL_USE_HPP
#define __PRO_SQL_USE_HPP


#include "binders.hpp"

namespace db { namespace mysql {


	namespace details 
	{
		struct use_param_t
		{
			enum_field_types buffer_type_;
			unsigned long buffer_length_ = 0;
			void *buffer_ = nullptr;
			bool is_unsigned_ = false;
			bool is_null_ = false;
			unsigned long length_ = 0; 
		};


		template <>
		struct cast_t<use_param_t>
		{
			template < std::uint32_t Index, std::uint32_t N, typename T >
			static void cast(std::array<use_param_t, N> &buffer, const T *val)
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
	using use_params_t = details::params_t<false, details::use_param_t, Args...>;

	template < typename ...Args >
	use_params_t<Args...> use(Args &...args)
	{
		return use_params_t<Args...>(args...);
	}

}}

#endif
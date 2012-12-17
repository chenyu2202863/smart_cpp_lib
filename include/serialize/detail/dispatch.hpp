#ifndef __SERIALIZE_DISPATCH_HPP
#define __SERIALIZE_DISPATCH_HPP


#include "container_dispatch.hpp"


namespace serialize
{
	// forward declare
	template < typename CharT, typename OutT >
	class serialize_t;


	namespace detail
	{

		namespace helper
		{
			template < typename CharT, typename OutT, typename T >
			void push_string(serialize_t<CharT, OutT> &os, const T *val)
			{
				const size_t bufLen = std::char_traits<T>::length(val);

				push_string(os, val, bufLen);
			}
			template < typename CharT, typename OutT, typename T >
			void push_string(serialize_t<CharT, OutT> &os, const T *val, size_t bufLen)
			{
				os.push(bufLen);
				os.push_pointer(val, bufLen);
			}
			template < typename CharT, typename OutT, typename T, size_t N >
			void push_string_array(serialize_t<CharT, OutT> &os, T (&val)[N])
			{
				push_string(os, val);
			}


			template < typename CharT, typename OutT, typename T >
			void pop_string(serialize_t<CharT, OutT> &os, T *val)
			{
				size_t bufLen = 0;
				os.pop(bufLen);
				os.pop_pointer(val, bufLen);
			}
			template < typename CharT, typename OutT, typename T, size_t N >
			void pop_string_array(serialize_t<CharT, OutT> &os, T (&val)[N])
			{
				size_t bufLen = 0;
				os.pop(bufLen);

				assert(bufLen <= N);
				if( bufLen > N )
					throw std::out_of_range("bufLen > N");

				os.pop_pointer(val, bufLen);
			}

		}


		template < typename CharT, typename OutT, typename T >
		void push_string(serialize_t<CharT, OutT> &os, const std::basic_string<T> &val)
		{
			helper::push_string(os, val.c_str(), val.length());
		}

		template < typename CharT, typename OutT, typename T >
		void pop_string(serialize_t<CharT, OutT> &os, std::basic_string<T> &val)
		{
			size_t bufLen = 0;
			os.pop(bufLen);

			val.resize(bufLen);
			os.pop_pointer(const_cast<T *>(val.data()), bufLen);
		}


		template < typename T >
		struct select_array_t
		{
			template < typename CharT, typename OutT, size_t N > 
			static void push_dispacth(serialize_t<CharT, OutT> &os, const T (&val)[N])
			{
				_push_dispacth(os, val, std::integral_constant<bool, std::is_pod<T>::value>());
			}

			template < typename CharT, typename OutT, size_t N > 
			static void pop_dispacth(serialize_t<CharT, OutT> &os, T (&val)[N])
			{
				_pop_dispacth(os, val, std::integral_constant<bool, std::is_pod<T>::value>());
			}

			template < typename CharT, typename OutT, size_t N > 
			static void _push_dispacth(serialize_t<CharT, OutT> &os, const T (&val)[N], std::true_type)
			{
				os.push_array(val);
			}

			template < typename CharT, typename OutT, size_t N > 
			static void _pop_dispacth(serialize_t<CharT, OutT> &os, T (&val)[N], std::true_type)
			{
				os.pop_array(val);
			}

			template < typename CharT, typename OutT, size_t N > 
			static void _push_dispacth(serialize_t<CharT, OutT> &os, const T (&val)[N], std::false_type)
			{
				for(size_t i = 0; i != N; ++i)
					os << val[i];
			}

			template < typename CharT, typename OutT, size_t N > 
			static void _pop_dispacth(serialize_t<CharT, OutT> &os, T (&val)[N], std::false_type)
			{
				for(size_t i = 0; i != N; ++i)
					os >> val[i];
			}
		};

		template < >
		struct select_array_t< char >
		{
			template < typename CharT, typename OutT, size_t N >
			static void push_dispacth(serialize_t<CharT, OutT> &os, const char (&val)[N])
			{
				helper::push_string_array(os, val);
			}

			template < typename CharT, typename OutT, size_t N >
			static void pop_dispacth(serialize_t<CharT, OutT> &os, char (&val)[N])
			{
				helper::pop_string_array(os, val);
			}
		};

		template < >
		struct select_array_t< wchar_t >
		{
			template < typename CharT, typename OutT, size_t N >
			static void push_dispacth(serialize_t<CharT, OutT> &os, const wchar_t (&val)[N])
			{
				helper::push_string_array(os, val);
			}

			template < typename CharT, typename OutT, size_t N >
			static void pop_dispacth(serialize_t<CharT, OutT> &os, wchar_t (&val)[N])
			{
				helper::pop_string_array(os, val);
			}
		};



		template < typename T >
		struct select_pointer_t
		{
			template < typename CharT, typename OutT > 
			static void push_dispacth(serialize_t<CharT, OutT> &os, const T &val)
			{
				_push_dispacth(os, val,
					std::integral_constant<bool, std::tr1::is_pod<typename std::remove_reference<T>::type>::value>());
			}

			template < typename CharT, typename OutT > 
			static void pop_dispacth(serialize_t<CharT, OutT> &os, T &val)
			{
				_pop_dispacth(os, val, 
					std::integral_constant<bool, std::tr1::is_pod<typename std::remove_reference<T>::type>::value>());
			}

			template < typename CharT, typename OutT > 
			static void _push_dispacth(serialize_t<CharT, OutT> &os, const T &val, std::true_type)
			{
				os.push(val);
			}

			template < typename CharT, typename OutT > 
			static void _pop_dispacth(serialize_t<CharT, OutT> &os, T &val, std::true_type)
			{
				os.pop(val);
			}

			template < typename CharT, typename OutT > 
			static void _push_dispacth(serialize_t<CharT, OutT> &os, const T &val, std::false_type)
			{
				value_traits_t<T>::push_dispatch(os, val);
			}

			template < typename CharT, typename OutT > 
			static void _pop_dispacth(serialize_t<CharT, OutT> &os, T &val, std::false_type)
			{
				value_traits_t<T>::pop_dispatch(os, val);
			}
		};

		template < typename T >
		struct select_pointer_t< T * >
		{
			template < typename CharT, typename OutT > 
			static void push_dispacth(serialize_t<CharT, OutT> &os, const T *val)
			{
				os << *val;
			}

			template < typename CharT, typename OutT > 
			static void pop_dispacth(serialize_t<CharT, OutT> &os, T *val)
			{
				os.pop_pointer(val);
			}
		};

		template < >
		struct select_pointer_t< char * >
		{
			template < typename CharT, typename OutT > 
			static void push_dispacth(serialize_t<CharT, OutT> &os, const char *val)
			{
				helper::push_string(os, val);
			}

			template < typename CharT, typename OutT > 
			static void pop_dispacth(serialize_t<CharT, OutT> &os, char *val)
			{
				helper::pop_string(os, val);
			}
		};

		template < >
		struct select_pointer_t< wchar_t * >
		{
			template < typename CharT, typename OutT > 
			static void push_dispacth(serialize_t<CharT, OutT> &os, const wchar_t *val)
			{
				helper::push_string(os, val);
			}

			template < typename CharT, typename OutT > 
			static void pop_dispacth(serialize_t<CharT, OutT> &os, wchar_t *val)
			{
				helper::pop_string(os, val);
			}
		};


		template < typename T >
		struct select_pointer_t< std::tr1::shared_ptr<T> >
		{
			template < typename CharT, typename OutT > 
			static void push_dispacth(serialize_t<CharT, OutT> &os, const std::shared_ptr<T> &val)
			{
				os << *val;
			}

			template < typename CharT, typename OutT > 
			static void pop_dispacth(serialize_t<CharT, OutT> &os, std::shared_ptr<T> &val)
			{
				val.reset(new T);
				os >> *val;
			}
		};

	}
}



#endif
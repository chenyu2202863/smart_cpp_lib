#ifndef __PRO_SQL_BINDERS_HPP
#define __PRO_SQL_BINDERS_HPP

#include "traits.hpp"


namespace db { namespace mysql { namespace details {

	template < typename T >
	struct cast_t;

	template < typename T >
	struct resize_t
	{
		static void resize(T &, std::uint32_t n)
		{
			assert(n == sizeof(T));
		}
	};

	template < >
	struct resize_t<std::string>
	{
		static void resize(std::string &val, std::uint32_t n)
		{
			if( val.capacity() < n )
				val.resize(n);
			else
				val._Eos(n);
		}
	};


	template < std::uint32_t ArrayIdx, std::uint32_t TupleIdx >
	struct tuple_for_each_t
	{
		template < typename ParamT, std::uint32_t N, typename ...Args >
		static void run(std::array<ParamT, N> &buffers, const std::tuple<Args...> &val)
		{
			using type = typename std::tuple_element<TupleIdx, std::tuple<Args...>>::type;

			cast_t<ParamT>::cast<ArrayIdx>(buffers, std::get<TupleIdx>(val));

			tuple_for_each_t<ArrayIdx - 1, TupleIdx - 1>::run(buffers, val);
		}

		template < typename ParamT, std::uint32_t N, typename ...Args >
		static void resize(std::array<ParamT, N> &buffers, const std::tuple<Args...> &val)
		{
			using type = typename std::tuple_element<TupleIdx, std::tuple<Args...>>::type;
			
			resize_t<std::remove_pointer<type>::type>::resize(*std::get<TupleIdx>(val), buffers[ArrayIdx].length_);

			tuple_for_each_t<ArrayIdx - 1, TupleIdx - 1>::resize(buffers, val);
		}
	};

	template < std::uint32_t ArrayIdx >
	struct tuple_for_each_t<ArrayIdx, 0>
	{
		template < typename ParamT, std::uint32_t N, typename ...Args >
		static void run(std::array<ParamT, N> &buffers, const std::tuple<Args...> &val)
		{
			using type = typename std::tuple_element<0, std::tuple<Args...>>::type;

			cast_t<ParamT>::cast<ArrayIdx>(buffers, std::get<0>(val));
		}

		template < typename ParamT, typename ...Args >
		static void run(std::array<ParamT, 0> &buffers, const std::tuple<Args...> &val)
		{}

		template < typename ParamT, std::uint32_t N, typename ...Args >
		static void resize(std::array<ParamT, N> &buffers, const std::tuple<Args...> &val)
		{
			using type = typename std::tuple_element<0, std::tuple<Args...>>::type;

			resize_t<std::remove_pointer<type>::type>::resize(*std::get<0>(val), buffers[ArrayIdx].length_);
		}
	};

	
	template < 
		bool IS_CONST, 
		typename ParamT,
		typename ...Args 
	>
	struct params_t
	{
		using tuple_t = typename std::conditional<IS_CONST, 
			std::tuple<const Args *...>,
			std::tuple<Args *...>>::type;
		static const std::uint32_t param_size = std::tuple_size<tuple_t>::value;

		using buffers_t = std::array<ParamT, param_size>;

		enum
		{
			ARRAY_SIZE = param_size == 0 ? 0 : param_size - 1,
			TUPLE_SIZE = ARRAY_SIZE
		};

		tuple_t params_;
		mutable buffers_t buffer_;

		params_t(Args &...args)
			: params_(std::make_tuple(&args...))
		{
			make_buffer();
		}

		params_t(const Args &...args)
			: params_(std::make_tuple(&args...))
		{
			make_buffer();
		}

		params_t(const params_t &) = delete;
		params_t &operator=(const params_t &) = delete;

		buffers_t &buffers()
		{
			return buffer_;
		}

		const buffers_t &buffers() const
		{
			return buffer_;
		}

		void resize()
		{
			details::tuple_for_each_t<ARRAY_SIZE, TUPLE_SIZE>::resize(buffer_, params_);
			make_buffer();
		}

		void finaly()
		{
			details::tuple_for_each_t<ARRAY_SIZE, TUPLE_SIZE>::resize(buffer_, params_);
		}

		void make_buffer()
		{
			details::tuple_for_each_t<ARRAY_SIZE, TUPLE_SIZE>::run(buffer_, params_);
		}
	};

}}}

#endif
#ifndef __EXTEND_STL_TYPE_TRAITS_HPP
#define __EXTEND_STL_TRAITS_HPP

#include <cassert>
#include <type_traits>
#include <utility>


namespace stdex
{


    //----------------------------------------------------------------------------------------------
    // aligned_union_t
    //
    template < typename ...Args >
    union aligned_union_t;

    template < typename T >
    union aligned_union_t<T>
    {
        typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type t;
    };

    template< typename T, typename ...Args >
    union aligned_union_t<T, Args...>
    {
        aligned_union_t<T> t;
        aligned_union_t<Args...> args;
    };


    //----------------------------------------------------------------------------------------------
    // aligned_storage_t
    //

    template < typename ...Args >
    class aligned_storage_t
    {
        aligned_union_t<Args...> storage_;

    public:
        void* address() { return &storage_; }

        void const* address() const { return &storage_; }
    };


    //----------------------------------------------------------------------------------------------
    // index_of_t
    //

    template < typename ...Args >
    struct index_of_t
    {
        static const std::size_t value = 0;
    };

    template < typename U, typename ...Args >
    struct index_of_t<U, U, Args...>
    {
        static const std::size_t value = 1;
    };

    template < typename U, typename T, typename ...Args >
    struct index_of_t<U, T, Args...>
    {
        static const std::size_t t = index_of_t<U, Args...>::value;
        static const std::size_t value = t == 0 ? 0 : 1 + t;
    };


    //----------------------------------------------------------------------------------------------
    // type_at_t
    //

    template < std::size_t I, typename ...Args >
    struct type_at_impl_t;

    template < typename T, typename ...Args >
    struct type_at_impl_t<1, T, Args...>
    {
        using type = T;
    };

    template < std::size_t I, typename T, typename ...Args >
    struct type_at_impl_t<I, T, Args...> 
		: type_at_impl_t<I - 1, Args...>
    {
    };

    template < std::size_t I, typename ...Args >
    using type_at_t = typename type_at_impl_t<I, Args...>::type;


    //----------------------------------------------------------------------------------------------
    // type_index_pair_t
    //

    template < typename T, std::size_t I >
    struct type_index_pair_t
    {
        using type = T;

        static const std::size_t index = I;
    };


    //----------------------------------------------------------------------------------------------
    // Overload
    //

    template < std::size_t I, typename ...Args >
    struct overload_t;

    template < std::size_t I >
    struct overload_t<I>
    {
        static void apply();
    };

    template < std::size_t I, typename T, typename ...Args > 
    struct overload_t<I, T, Args...>
		: overload_t<I + 1, Args...>
    {
        using base = overload_t<I + 1, Args...>;
        using base::apply;

        static type_index_pair_t<T, I> apply(T const&);
    };


    template < typename OverloadType, typename A >
    using overload_resolution_t = decltype( OverloadType::apply(std::declval<A>()) );



    template < typename ...Ts >
    struct first_impl_t;

    template < typename T, typename ...Args >
    struct first_impl_t<T, Args...>
    {
        using type = T;
    };

    template < typename ...Args >
    using first_t = typename first_impl_t<Args...>::type;


    template < typename F, typename ...A >
    using result_of_t = typename std::result_of<F(A...)>::type;


    template < typename T, typename U = void >
    using enable_if_t = typename std::enable_if<T::value, U>::type;


    template < typename T, typename U = void >
    using disable_if_t = typename std::enable_if<!T::value, U>::type;


    template < typename Condition, typename True, typename False >
    using conditional_t = typename std::conditional<Condition::value, True, False>::type;


    template < typename T >
    using decay_t = typename std::decay<T>::type;


    template < typename T, typename U >
    using equivalent_t = typename std::is_same<decay_t<T>, decay_t<U>>::type;


    template < typename From, typename To >
    using convertible_t = typename std::is_convertible<From, To>::type;


    template < typename ...Args >
    struct all_t 
		: std::false_type
    {
    };

    template < typename T >
    struct all_t<T> 
		: conditional_t<T, std::true_type, std::false_type>
    {
    };

    template < typename T, typename ...Args >
    struct all_t<T, Args...> 
		: conditional_t<T, all_t<Args...>, std::false_type>
    {
    };


    template < typename ...Args >
    struct any_t 
		: std::false_type
    {
    };

    template < typename T >
    struct any_t<T> 
		: conditional_t<T, std::true_type, std::false_type>
    {
    };

    template < typename T, typename ...Args >
    struct any_t<T, Args...> 
		: conditional_t<T, std::true_type, any_t<Args...>>
    {
    };


    //----------------------------------------------------------------------------------------------
    // is_equality_comparable_t
    //


    template < typename T, typename U >
    struct is_equality_comparable_impl_t
    {
        template<typename X> static std::true_type Test(convertible_t<decltype(std::declval<X>() == std::declval<U>()), bool>*);
        template<typename X> static std::false_type Test(...);

        using type = decltype(Test<T>(0));
    };

    template < typename T, typename U >
    struct is_equality_comparable_t 
		: is_equality_comparable_impl_t<T, U>::type
    {
    };

	

	//----------------------------------------------------------------------------------------------
	// Cast
	//

	template < typename T >
	struct cast_t 
	{
		static T* apply(void* Pointer)
		{
			return static_cast<T*>(Pointer);
		}

		static T const* apply(void const* Pointer)
		{
			return static_cast<T const*>(Pointer);
		}
	};


	//----------------------------------------------------------------------------------------------
	// Visit
	//

	template < typename R, typename V, typename ...Args >
	struct visit_t;

	template < typename R, typename V >
	struct visit_t<R, V> 
	{
		template < typename P, typename ...A >
		static R apply(std::size_t, P, V&&, A&&...)
		{
			assert(!"unreachable");
		}
	};

	template < typename R, typename V, typename T, typename ...Args >
	struct visit_t<R, V, T, Args...> 
	{
		template < typename P, typename ...A >
		static R apply(std::size_t Index, P Pointer, V&& visitor, A&&... args)
		{
			if( Index == 1 )
			{
				if( auto ptr = cast_t<T>::apply(Pointer) )
				{
					return visitor(*ptr, std::forward<A>(args)...);
				}

				throw std::runtime_error("visit moved-from recursive variant");
			}

			return visit_t<R, V, Args...>::apply(
				Index - 1,
				Pointer,
				std::forward<V>(visitor),
				std::forward<A>(args)...
				);
		}
	};

	namespace details {

		template <typename T>
		struct function_traits_t
			: public function_traits_t<decltype(&T::operator( ))>
		{};


		namespace xx_impl {
			template <typename C, typename R, typename... A>
			struct memfn_type
			{
				typedef typename std::conditional <
				std::is_const<C>::value, typename std::conditional <std::is_volatile<C>::value, R(C::*)(A...) const volatile, R(C::*)(A...) const> ::type,
				typename std::conditional <std::is_volatile<C>::value, R(C::*)(A...) volatile, R(C::*)(A...)> ::type
				> ::type type;
			};
		}


		template <typename R, typename... Args>
		struct function_traits_t<R(Args...)>
		{
			typedef R result_type;
			typedef std::tuple<Args...> args_type;
			typedef R function_type(Args...);

			template <typename OwnerType>
			struct member_function_type
				: xx_impl::memfn_type <
				typename std::remove_pointer<typename std::remove_reference<OwnerType>::type>::type,
				R, Args...
				> ::type
			{};

			enum
			{
				arity = sizeof...(Args)
			};

			template <size_t i>
			struct arg
			{
				typedef typename std::tuple_element<i, args_type>::type type;
			};
		};


		template <typename R, typename... Args>
		struct function_traits_t<R(*)(Args...)>
			: public function_traits_t<R(Args...)>
		{};


		template <typename C, typename R, typename... Args>
		struct function_traits_t<R(C::*)(Args...)>
			: public function_traits_t<R(Args...)>
		{
			typedef C& owner_type;
		};


		template <typename C, typename R, typename... Args>
		struct function_traits_t<R(C::*)(Args...) const>
			: public function_traits_t<R(Args...)>
		{
			typedef const C& owner_type;
		};


		template <typename C, typename R, typename... Args>
		struct function_traits_t<R(C::*)(Args...) volatile>
			: public function_traits_t<R(Args...)>
		{
			typedef volatile C& owner_type;
		};


		template <typename C, typename R, typename... Args>
		struct function_traits_t<R(C::*)(Args...) const volatile>
			: public function_traits_t<R(Args...)>
		{
			typedef const volatile C& owner_type;
		};


		template <typename FunctionType>
		struct function_traits_t<std::function<FunctionType>>
			: public function_traits_t<FunctionType>
		{};


		template <typename T>
		struct function_traits_t<T&> : public function_traits_t<T>{};
		template <typename T>
		struct function_traits_t<const T&> : public function_traits_t<T>{};
		template <typename T>
		struct function_traits_t<volatile T&> : public function_traits_t<T>{};
		template <typename T>
		struct function_traits_t<const volatile T&> : public function_traits_t<T>{};
		template <typename T>
		struct function_traits_t<T && > : public function_traits_t<T>{};
		template <typename T>
		struct function_traits_t<const T && > : public function_traits_t<T>{};
		template <typename T>
		struct function_traits_t<volatile T && > : public function_traits_t<T>{};
		template <typename T>
		struct function_traits_t<const volatile T && > : public function_traits_t<T>{};


		// is_callable
		template<typename Func, typename... Args, typename = decltype(std::declval<Func>()(std::declval<Args>()...))>
		std::true_type is_callable_helper(int);

		template<typename Func, typename... Args>
		std::false_type is_callable_helper(...);

	}
	
	template<typename T, typename ...Args >
	struct is_callable;

	template<typename Func, typename... Args>
	struct is_callable<Func(Args...)>
		: public decltype(details::is_callable_helper<Func, Args...>(0)) {};

	template < typename T, typename ...Args >
	struct is_callable<T, std::tuple<Args...>>
		: is_callable<T(Args...)>
	{};
} 


#endif
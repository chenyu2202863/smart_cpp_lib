#ifndef __EXTEND_STL_VARIANT_HPP
#define __EXTEND_STL_VARIANT_HPP


#include "../type_traits.hpp"

#include <stdexcept>


namespace stdext { namespace container {


	template < typename ...Args >
	class variant_t
	{
		template<typename A>
		using target_t = overload_resolution_t< overload_t<1, Args...>, A >;

		using first_type = first_t<Args...>;

		// storage for a value of any of the types
		aligned_storage_t<Args...> storage_;
		// based index of the currently hold value, 1--hold,0--no
		size_t index_;

	public:
		variant_t()
			: index_(0)
		{}

		variant_t(variant_t const& rhs)
			: index_(0)
		{
			if( rhs.index_ != 0 )
				_apply<void>(rhs, copy_visitor_t(), *this);
		}

		variant_t(variant_t&& rhs)
			: index_(0)
		{
			if( rhs.Index != 0 )
				_apply<void>(rhs, move_visitor_t(), *this);
		}

		template<typename U, typename V = target_t<U>>
		variant_t(U&& Arg)
			: index_(0)
		{
			_construct<typename V::type>(V::index, std::forward<U>(Arg));
		}

		variant_t& operator =(variant_t const& rhs)
		{
			if( rhs.index_ != 0 )
				_apply<void>(rhs, copy_visitor_t(), *this);
			else
				_destroy();

			return *this;
		}

		variant_t& operator =(variant_t&& rhs)
		{
			if( rhs.Index != 0 )
				_apply<void>(rhs, move_visitor_t(), *this);
			else
				_destroy();

			return *this;
		}

		template<typename U, typename V = target_t<U>>
		variant_t& operator =(U&& Arg)
		{
			_assign_or_construct<typename V::type>(V::index, std::forward<U>(Arg));
			return *this;
		}

		~variant_t()
		{
			_destroy();
		}

		size_t index() const
		{
			return index_;
		}

		bool empty() const
		{
			return index_ == 0;
		}

		template<typename V, typename ...A, typename R = result_of_t<V, first_type&, A...>>
		R apply(V&& Visitor, A&&... Args)
		{
			return _apply<R>(*this, std::forward<V>(Visitor), std::forward<A>(Args)...);
		}

		template<typename V, typename ...A, typename R = result_of_t<V, first_type const&, A...>>
		R apply(V&& visitor, A&&... args) const
		{
			return _apply<R>(*this, std::forward<V>(visitor), std::forward<A>(args)...);
		}

		template<typename U>
		U* get()
		{
			return get_t<U>::apply(index_, storage_.address());
		}

		template<typename U>
		U const* get() const
		{
			return const_cast<variant_t*>(this)->template get<U>();
		}

		template<typename U>
		U& as()
		{
			auto Pointer = get<U>();
			if( Pointer )
				return *Pointer;

			throw std::runtime_error("bad cast");
		}

		template<typename U>
		U const& as() const
		{
			return const_cast<variant_t*>(this)->template as<U>();
		}

		template<typename U, typename V = target_t<U>>
		bool equals(U&& rhs) const
		{
			if( index_ == V::index )
				return *get<typename V::type>() == std::forward<U>(rhs);

			return false;
		}

		bool equals(variant_t const& rhs) const
		{
			bool L = !empty();
			bool R = !rhs.empty();

			return (L && R) ? apply(equals_visitor_t(), rhs) : (L == R);
		}

	private:
		template<typename R, typename U, typename V, typename ...A>
		static R _apply(U& self, V&& visitor, A&&... args)
		{
			if( self.index_ == 0 )
				throw std::runtime_error("visit empty variant");

			return visit_t<R, V, Args...>::apply(
				self.index_,
				self.storage_.address(),
				std::forward<V>(visitor),
				std::forward<A>(args)...
				);
		}

		template<typename U, typename A>
		void _construct(size_t I, A&& Arg)
		{
			::new (storage_.address()) U(std::forward<A>(Arg));

			index_ = I;
		}

		template<typename U, typename A>
		void _assign(A&& Arg)
		{
			U& Ref = *static_cast<U*>(storage_.address());

			Ref = std::forward<A>(Arg);
		}

		template<typename U, typename A>
		void _reconstruct(size_t I, A&& Arg, std::true_type)
		{
			_destroy();
			_construct<U>(I, std::forward<A>(Arg));
		}

		template<typename U, typename A>
		void _reconstruct(size_t I, A&& Arg, std::false_type)
		{
			U Temp(std::forward<A>(Arg));

			_destroy();
			_construct<U>(I, std::move(Temp));
		}

		template<typename U, typename A, typename Tag>
		void _assign_or_construct(size_t I, A&& Arg, Tag)
		{
			if( index_ == I )
				_assign<U>(std::forward<A>(Arg));
			else if( index_ == 0 )
				_construct<U>(I, std::forward<A>(Arg));
			else
				_reconstruct<U>(I, std::forward<A>(Arg), Tag());
		}

		template<typename U, typename A>
		void _assign_or_construct(size_t I, A&& Arg)
		{
			_assign_or_construct<U>(I, std::forward<A>(Arg), std::is_nothrow_constructible<U, A>());
		}

		void _destroy()
		{
			if( index_ != 0 )
				_apply<void>(*this, destroy_visitor_t());
		}

		// Internal!
		struct copy_visitor_t
		{
			template<typename U>
			void operator ()(U const& rhs, variant_t& LHS) const
			{
				LHS._assign_or_construct<U>(
					index_of_t<U, Args...>::value,
					rhs,
					std::is_nothrow_copy_constructible<U>()
					);
			}
		};

		// Internal!
		struct move_visitor_t
		{
			template<typename U>
			void operator ()(U& rhs, variant_t& LHS) const
			{
				LHS._assign_or_construct<U>(
					index_of_t<U, Args...>::value,
					std::move(rhs),
					std::true_type() // assume all Args have non-throwing move-ctor and move-assign
					);
			}
		};

		// Internal!
		struct destroy_visitor_t
		{
			template<typename U>
			void operator ()(U& Value) const
			{
				Value.U::~U();
			}
		};

		// External!
		struct equals_visitor_t
		{
			template<typename U>
			bool operator ()(U const& LHS, variant_t const& rhs) const
			{
				return rhs.equals(LHS);
			}
		};

		template< typename U, size_t I = index_of_t<U, Args...>::value >
		struct get_t
		{
			static U* apply(size_t Index, void* Pointer)
			{
				if( Index != I )
					return 0;

				return cast_t<type_at_t<I, Args...>>::apply(Pointer);
			}
		};

		template< typename U>
		struct get_t<U, 0>
		{
			static U* apply(size_t, void*)
			{
				return 0;
			}
		};
	};


	template<typename ...Args, typename U>
	inline bool operator ==(variant_t<Args...> const& LHS, U&& rhs)
	{
		static_assert(std::is_constructible<variant_t<Args...>, U>::value,
			"invalid comparison");

		return LHS.equals(std::forward<U>(rhs));
	}


	template<typename ...Args, typename U>
	inline bool operator !=(variant_t<Args...> const& LHS, U&& rhs)
	{
		return !(LHS == std::forward<U>(rhs));
	}


	template < typename U, typename ...Args, typename = disable_if_t < equivalent < U, variant_t < Args... >> >>
		inline bool operator ==(U&& LHS, variant_t<Args...> const& rhs)
	{
		return rhs == std::forward<U>(LHS);
	}


	template < typename U, typename ...Args, typename = disable_if_t < equivalent < U, variant_t < Args... >> >>
		inline bool operator !=(U&& LHS, variant_t<Args...> const& rhs)
	{
		return !(std::forward<U>(LHS) == rhs);
	}


}}

#endif

#ifndef __LUA_REFERENCE_HPP
#define __LUA_REFERENCE_HPP

#include "config.hpp"
#include "state.hpp"
#include "error.hpp"

namespace luareg {


	namespace details 
	{
		struct function_check_t
		{
			static bool check(state_t &state, int ref)
			{
				::lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
				LUAREG_ERROR(lua_isfunction(state, -1), LUA_TFUNCTION, ref);
				lua_pop(state, 1);
				return true;
			}
		};

		struct table_check_t
		{
			static bool check(state_t &state, int ref)
			{
				::lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
				LUAREG_ERROR(lua_istable(state, -1), LUA_TTABLE, ref);
				lua_pop(state, 1);

				return true;
			}
		};

		struct string_check_t
		{
			static bool check(state_t &state, int ref)
			{
				::lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
				LUAREG_ERROR(::lua_isstring(state, -1) == 0, LUA_TSTRING, ref);
				lua_pop(state, 1);

				return true;
			}
		};
	}
	

	template < typename CheckT >
	class reference_t
		: CheckT
	{
		lua_State *state_;
		int ref_;

	public:
		reference_t()
			: state_(nullptr)
			, ref_(LUA_NOREF)
		{}

		reference_t(lua_State *state)
			: state_(state)
			, ref_(LUA_NOREF)
		{
			ref_ = ::luaL_ref(state_, LUA_REGISTRYINDEX);
			assert(ref_ != LUA_REFNIL || ref_ != LUA_NOREF);
		}
		
		~reference_t() 
		{
			clear();
		}

		reference_t(reference_t && rhs)
			: ref_(rhs.ref_)
			, state_(rhs.state_)
		{
			rhs.ref_ = LUA_NOREF;
			rhs.state_ = nullptr;
		}

		reference_t &operator=(reference_t &&rhs)
		{
			if(this != &rhs)
			{
				ref_ = rhs.ref_;
				state_ = rhs.state_;

				rhs.ref_ = LUA_NOREF;
				rhs.state_ = nullptr;
			}

			return *this;
		}

		reference_t(const reference_t &) = delete;
		reference_t &operator=(const reference_t &) = delete;


	public:
		bool is_valid() const
		{
			return state_ != nullptr &&
				ref_ != LUA_NOREF &&
				CheckT::check(state_t(state_), ref_);
		}

		void get() const
		{
			assert(state_ != nullptr);
			assert(ref_ != LUA_NOREF);

			::lua_rawgeti(state_, LUA_REGISTRYINDEX, ref_);
		}

		void clear()
		{
			if( ref_ != LUA_NOREF )
			{
				::luaL_unref(state_, LUA_REGISTRYINDEX, ref_);
				ref_ = LUA_NOREF;
			}
		}

		lua_State *state() const
		{
			return state_;
		}
	};

	typedef reference_t<details::function_check_t>	function_ref_t;
	typedef reference_t<details::table_check_t>		table_ref_t;
	typedef reference_t<details::string_check_t>	string_ref_t;
}

#endif
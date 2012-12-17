#ifndef __UTILITY_SCOPE_EXIT_HPP
#define __UTILITY_SCOPE_EXIT_HPP

/** @ini.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* 利用RAII机制，退出作用域调用指定过程
*/

#include <functional>
#include <memory>


namespace utility
{

	/**
	* @class <scope_exit_t>
	* @brief 
	*
	* 利用C++RAII机制，构造时执行InitFunct，析构时执行UninitFuncT
	*/
	template < typename InitFuncT, typename UninitFuncT >
	class scope_exit_t
	{
		typedef InitFuncT		InitFuncionType;
		typedef UninitFuncT		UninitFunctionType;

	private:
		InitFuncionType			initFunc_;		// 初始化
		UninitFunctionType		unInitFunc_;	// 反初始化

	public:
		/**
		* @brief 构造函数，需要传入初始化执行过程和反初始化的执行过程
		* @param <init> <初始化执行过程>
		* @param <uninit> <反初始化执行过程>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		scope_exit_t(const InitFuncionType &init, const UninitFunctionType &unInt)
			: initFunc_(init)
			, unInitFunc_(unInt)
		{
			initFunc_();
		}
		~scope_exit_t()
		{
			unInitFunc_();
		}

		scope_exit_t(scope_exit_t &&rhs)
			: initFunc_(std::move(rhs.initFunc_))
			, unInitFunc_(std::move(rhs.unInitFunc_))
		{}

		scope_exit_t &operator=(scope_exit_t &&rhs)
		{
			if( &rhs != this )
			{
				initFunc_ = std::move(rhs.initFunc_);
				unInitFunc_ = std::move(rhs.unInitFunc_);
			}
		}

	private:
		scope_exit_t(const scope_exit_t &);
		scope_exit_t &operator=(const scope_exit_t &);
	};


	/**
	* @brief利用编译期类型推导创建scope_exit对象
	* @param <init> <初始化执行过程>
	* @param <uninit> <反初始化执行过程>
	* @exception <无任何异常抛出>
	* @return <无>
	* @note <无>
	* @remarks <无>
	*/
	template < typename InitT, typename UninitT >
	inline scope_exit_t<InitT, UninitT> make_scope_exit(const InitT &init, const UninitT &uninit)
	{
		return scope_exit_t<InitT, UninitT>(init, uninit);
	}
}




#endif
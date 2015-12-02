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
	template < typename D >
	auto make_scope_exit(D &&d)
	{
		auto deletor = [](D *p)
		{
			(*p)();
		};
		return std::unique_ptr<D, decltype(deletor)>(&d, deletor);
	}
}




#endif
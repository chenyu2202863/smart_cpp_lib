#ifndef __WIN32_DLL_DYNAMIC_HPP
#define __WIN32_DLL_DYNAMIC_HPP

#include <cassert>
#include <type_traits>
#include <Windows.h>

namespace sys {
	

	// -------------------------

	class dynamic_function_base
	{
	protected:
		HMODULE handle_ = nullptr;

	public:
		explicit dynamic_function_base(const wchar_t* dllName)
			: handle_(0)
		{
			handle_ = ::LoadLibrary(dllName);
			assert(handle_ != nullptr);
		}

		dynamic_function_base::~dynamic_function_base()
		{
			if( handle_ )
				::FreeLibrary(handle_);
		}

		bool is_valid() const
		{
			return handle_ != nullptr;
		}

		dynamic_function_base(const dynamic_function_base&) = delete;
		dynamic_function_base operator=(const dynamic_function_base&) = delete;
	};



	struct dynamic_function_t
		: public dynamic_function_base
	{
		struct ret_helper_t
		{
			void *ptr_ = nullptr;
			ret_helper_t(void *ptr)
				: ptr_(ptr)
			{}

			template< typename T >
			operator T()
			{
				using function_t = std::remove_pointer<T>::type;
				static_assert(std::is_function<function_t>::value, "T must be function type");
				
				return (T)ptr_;
			}
		};

	public:
		dynamic_function_t(const wchar_t* dllName)
			: dynamic_function_base(dllName)
		{}

		
		ret_helper_t get(const char* fnName) const
		{
			assert(is_valid());

			void *func_ptr = ::GetProcAddress(handle_, fnName);
			if( !func_ptr )
			{
				assert(0);
				return 0;
			}

			return ret_helper_t{func_ptr};
		}
	};

}




#endif
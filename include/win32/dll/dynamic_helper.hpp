#ifndef __WIN32_DLL_DYNAMIC_HPP
#define __WIN32_DLL_DYNAMIC_HPP

#include <cassert>
#include <Windows.h>

namespace win32
{
	namespace dll
	{

		// -------------------------

		class dynamic_function_base 
		{
		protected:
			HMODULE handle_;

		public:
			explicit dynamic_function_base(const TCHAR* dllName) 
				: handle_(0)
			{
				handle_ = ::LoadLibrary(dllName);
				assert(handle_ != 0);
			}

			dynamic_function_base::~dynamic_function_base() 
			{
				if (handle_)
					::FreeLibrary(handle_);
			}

			bool is_valid() const
			{
				return handle_ != 0;
			}

		private:
			dynamic_function_base(const dynamic_function_base&);
			dynamic_function_base operator=(const dynamic_function_base&);
		};


		
		class dynamic_function_t 
			: public dynamic_function_base 
		{
		public:
			dynamic_function_t(const TCHAR* dllName) 
				: dynamic_function_base(dllName) 
			{}

			template< typename T > 
			T get(const char* fnName) const 
			{ 
				assert( is_valid() );

				void *func_ptr = ::GetProcAddress(handle_, fnName);
				if( !func_ptr )
				{
					assert(0);
					return 0;
				}

				return (T)func_ptr; 
			};
		};
	}
}




#endif
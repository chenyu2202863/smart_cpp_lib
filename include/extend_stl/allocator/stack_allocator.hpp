#ifndef __EXTEND_STL_STACK_ALLOCATOR_HPP
#define __EXTEND_STL_STACK_ALLOCATOR_HPP

#include <type_traits>
#include <cassert>
#include <array>
#include <cstdint>

namespace stdex { namespace allocator {


	template<std::size_t n_stack_elements>
	class stack_storage
	{
	public:
		typedef typename std::aligned_storage<sizeof(char), std::alignment_of<char>::value>::type storage_type;
		stack_storage()
			: pos_(0)
		{
		}

		std::uint32_t pos_;
		storage_type array_[n_stack_elements];
	};

	template<typename T, std::size_t n_stack_elements>
	class stack_allocator_t
		: public std::allocator<T>
	{
		typedef std::allocator<T> base_class;
		
	public:
		template<typename U>
		struct rebind
		{
			typedef stack_allocator_t<U, n_stack_elements> other;
		};

		stack_storage<n_stack_elements> &storage_;


		explicit stack_allocator_t(stack_storage<n_stack_elements> &storage)
			: storage_(storage)
		{
		}

		template < typename U >
		stack_allocator_t(stack_allocator_t<U, n_stack_elements> &rhs)
			: storage_(rhs.storage_)
		{}

		typename base_class::pointer allocate(typename base_class::size_type n_elements,
			std::allocator<void>::const_pointer hint = 0)
		{
			assert(storage_.pos_ + n_elements * sizeof(base_class::value_type) <= n_stack_elements);
			if( storage_.pos_ + n_elements * sizeof(base_class::value_type) > n_stack_elements )
				return nullptr;
			else
			{
				base_class::pointer p = reinterpret_cast<base_class::pointer>(&storage_.array_[storage_.pos_]);
				storage_.pos_ += n_elements * sizeof(base_class::value_type);
				return p;
			}
		}
		void deallocate(typename base_class::pointer p, typename base_class::size_type n)
		{
		}	
	};

}


}


#endif


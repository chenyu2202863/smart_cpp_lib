#ifndef __UTILITY_UNION_CAST_HPP
#define __UTILITY_UNION_CAST_HPP

namespace utility {

	template < typename T >
	struct union_cast_t
	{
		const T &val_;

		union_cast_t(const T &val)
			: val_(val)
		{}

		template < typename U >
		operator U()
		{
			union
			{
				T src_;
				U dest_;
			};
			
			src_ = val_;
			return dest_;
		}
	};

	template < typename T >
	union_cast_t<T> union_cast(const T &t)
	{
		return union_cast_t<T>(t);
	}
}
#endif
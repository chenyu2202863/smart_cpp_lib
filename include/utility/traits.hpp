#ifndef __UTILITY_TRAITS_HPP
#define __UTILITY_TRAITS_HPP



#define CREATE_MEMBER_DETECTOR(X)                                                   \
namespace utility {																	\
	template<typename T>															\
	class detect_##X																\
	{																				\
	struct Fallback { int X; };                                                     \
	struct Derived : T, Fallback { };                                               \
																					\
	template<typename U, U> struct Check;                                           \
																					\
	typedef char ArrayOfOne[1];                                                     \
	typedef char ArrayOfTwo[2];                                                     \
																					\
	template<typename U> static ArrayOfOne & func(Check<int Fallback::*, &U::X> *); \
	template<typename U> static ArrayOfTwo & func(...);                             \
	public:                                                                         \
	typedef detect_##X type;                                                        \
	enum { value = sizeof(func<Derived>(0)) == 2 };                                 \
};																					\
}


#endif
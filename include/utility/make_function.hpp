#ifndef __UTILITY_MAKE_FUNCTION_HPP
#define __UTILITY_MAKE_FUNCTION_HPP


namespace utility {

	template < typename HandlerT, typename AllocatorT >
	auto make_function(HandlerT &&handler, const AllocatorT &allocator)
		->std::function<typename luareg::details::function_traits_t<HandlerT>::function_type>
	{
		using function_t = luareg::details::function_traits_t<HandlerT>;

		return std::function<typename function_t::function_type>(std::forward<HandlerT>(handler), allocator);
	}

}
#endif
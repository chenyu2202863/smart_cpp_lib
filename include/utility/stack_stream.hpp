#ifndef __UTILITY_STACK_STREAM_HPP
#define __UTILITY_STACK_STREAM_HPP


#include <streambuf>
#include <ostream>
#include <cassert>
#include <cstdint>

namespace utility {

	template < std::uint32_t OnStackSize = 1024 >
	class stack_buffer_t : public std::streambuf 
	{
		char on_stack_[OnStackSize + 1];

	public:
		stack_buffer_t() 
		{
			init();
		}
		~stack_buffer_t()
		{
		}

	private:
		stack_buffer_t(stack_buffer_t const &);
		stack_buffer_t &operator=(stack_buffer_t const &);

	public:
		const char *begin() const
		{
			return pbase();
		}

		const char *end() const
		{
			return pptr();
		}

		const char *c_str() const
		{
			*pptr() = 0;
			return begin();
		}
	
		std::string str()
		{
			return std::string(begin(),size_t(end()-begin()));
		}
		
	protected:
		int overflow(int c)
		{
			assert(0);
			return 0;
		}

	private:
		void init()
		{
			setp(on_stack_,on_stack_+OnStackSize);
		}
		
	};



	template < std::uint32_t Size = 1024 >
	class stack_stream_impl_t
		: public std::ostream 
	{
		stack_buffer_t<Size> buf_;

	public:
		stack_stream_impl_t() 
			: std::ostream(0)
		{
			rdbuf(&buf_);
		}

		//stack_stream_impl_t(stack_stream_impl_t &&rhs)
		//	: 

		const char *begin() const
		{
			return buf_.begin();
		}

		const char *end() const
		{
			return buf_.end();
		}

		const char *c_str() const
		{
			return buf_.c_str();
		}

		std::uint32_t size() const
		{
			return (std::uint32_t)(buf_.end() - buf_.begin());
		}

		std::string str()
		{
			return buf_.str();
		}
	};

	struct stack_stream_t
	{
		stack_stream_impl_t<1024> os_;

	public:
		stack_stream_t() = default;
		//stack_stream_t(stack_stream_t &&rhs)
		//	: os_(std::move(rhs.os_))
		//{}


	private:
		stack_stream_t(const stack_stream_t &);
		stack_stream_t &operator=(const stack_stream_t &);

	public:
		template < typename T >
		stack_stream_t &operator<<(const T &t)
		{
			os_ << t;
			return *this;
		}

		stack_stream_t &operator<<(const std::pair<const char *, std::uint32_t> &val)
		{
			return write(val.first, val.second);
		}

		stack_stream_t &write(const char *buf, std::uint32_t len)
		{
			os_.write(buf, len);
			return *this;
		}

		operator const char *() const
		{
			return str().first;
		}

		std::pair<const char *, std::uint32_t> str() const
		{
			return std::make_pair(os_.c_str(), (std::uint32_t)os_.size());
		}
	};
}

#endif
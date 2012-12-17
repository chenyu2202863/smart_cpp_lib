#ifndef __DATABASE_SERIALIZE_STREAM_HPP
#define __DATABASE_SERIALIZE_STREAM_HPP

#include <cassert>
#include <type_traits>


namespace database {
	
	struct ostream
	{
		char *buf_;
		size_t len_;
		size_t index_;

		ostream(char *buf, size_t len)
			: buf_(buf)
			, len_(len)
			, index_(0)
		{}

		template < typename T >
		typename std::enable_if<std::is_pod<T>::value, ostream>::type &operator<<(const T &val)
		{
			static_assert(std::is_pod<T>::value, "must pod type");
			assert(index_ + sizeof(T) <= len_);

			if( index_ + sizeof(T) > len_ )
				throw std::out_of_range("out of range");

			memcpy(buf_ + index_, &val, sizeof(T));
			index_ += sizeof(T);

			return *this;
		}

		ostream &write(const void *buf, size_t len)
		{
			assert(index_ + len <= len_);

			if( index_ + len > len_ )
				throw std::out_of_range("out of range");

			memcpy(buf_ + index_, buf, len);
			index_ += len;

			return *this;
		}

		size_t size() const
		{
			return index_;
		}
	};



	struct istream
	{
		const char *buf_;
		size_t len_;
		size_t index_;

		istream(const char *buf, size_t len)
			: buf_(buf)
			, len_(len)
			, index_(0)
		{}

		template < typename T >
		typename std::enable_if<std::is_pod<T>::value, istream>::type &operator>>(T &val)
		{
			static_assert(std::is_pod<T>::value, "must pod type");
			assert(len_ - index_ >= sizeof(T));

			if( len_ - index_ < sizeof(T) )
				throw std::out_of_range("out of range");

			memcpy(&val, buf_ + index_, sizeof(T));
			index_ += sizeof(T);

			return *this;
		}

		istream &read(void *buf, size_t len)
		{
			assert(len_ - index_ >= len);

			if( len_ - index_ < len )
				throw std::out_of_range("out of range");

			memcpy(buf, buf_ + index_, len);
			index_ += len;

			return *this;
		}

		size_t size() const
		{
			return index_;
		}
	};
}




#endif
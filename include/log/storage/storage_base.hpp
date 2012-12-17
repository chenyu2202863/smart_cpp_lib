#ifndef __LOG_STORAGE_BASE_HPP
#define __LOG_STORAGE_BASE_HPP


namespace log_system
{
	namespace storage
	{
		//--------------------------------------------------------------------------
		// class storage_base_t

		template < typename CharT >
		struct storage_base_t
		{
			typedef CharT	char_type;
			typedef size_t	size_type;
		};


		// -------------------------------------------------------------------------
		// interface ILogStorage

		template < typename CharT >
		struct i_storage_t
		{
			typedef CharT	char_type;
			typedef size_t	size_type;

		public:
			virtual ~i_storage_t() {}

			virtual void put(char_type ch) = 0;
			virtual void put(size_t szCount, char_type ch) = 0;
			virtual void put(const char_type *pStr, size_t szCount) = 0;
			virtual void put(const char_type *fmt, va_list args) = 0;

			virtual void flush() = 0;
		};
	}
}


#endif
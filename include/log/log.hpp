#ifndef __LOG_BASIC_HPP
#define __LOG_BASIC_HPP

#include <deque>

#include "storage/storage.hpp"
#include "storage/cache_storage.hpp"
#include "storage/async_storage.hpp"

#include "log/log.hpp"
#include "log/thread_safe_log.hpp"

#include "filter/priority.hpp"
#include "format/format.hpp"




namespace log_system
{
	// ----------------------------------------------------------
	// storage

	typedef storage::file_storage_t<char>								file_storage;
	typedef storage::file_storage_t<wchar_t>							w_file_storage;

	typedef storage::debug_storage_t<char>								debug_storage;
	typedef storage::debug_storage_t<wchar_t>							w_debug_storage;

	typedef storage::console_storage_t<char>							console_storage;
	typedef storage::console_storage_t<wchar_t>							w_console_storage;

	typedef storage::string_storage_t<char>								string_storage;
	typedef storage::string_storage_t<wchar_t>							w_string_storage;

	typedef storage::cache_storage_t<512, file_storage>					cache_file_storage;
	typedef storage::async_storage_t<file_storage>						async_file_storage;


	// -----------------------------------------------------
	// logsystem
	using detail::log_t;

	typedef detail::log_t<file_storage, filter::default_filter>			file_log;
	typedef detail::log_t<w_file_storage, filter::default_filter>		w_file_log;

	typedef detail::log_t<debug_storage, filter::default_filter>		debug_log;
	typedef detail::log_t<w_debug_storage, filter::default_filter>		w_debug_log;

	typedef detail::log_t<console_storage, filter::default_filter>		console_log;
	typedef detail::log_t<w_console_storage, filter::default_filter>	w_console_log;

	typedef detail::log_t<string_storage, filter::default_filter>		string_log;
	typedef detail::log_t<w_string_storage, filter::default_filter>		w_string_log;

	typedef detail::log_t<cache_file_storage, filter::default_filter>	cache_file_log;

	typedef detail::log_t<async_file_storage, filter::default_filter>	async_file_log;



	// thread safe
	using detail::safe;

	// ---------------------------------------------------------------------
	// MultiLog

	typedef storage::file_storage_t<char, true, storage::i_storage_t>		multi_file_storage;
	typedef storage::debug_storage_t<char, storage::i_storage_t>			multi_debug_storage;
	typedef storage::string_storage_t<char, storage::i_storage_t>			multi_string_storage;

	using detail::multi_log_t;

	
	// format
	using format::ends;
	using format::endl;
	using format::level;
	using format::clr;
	using format::space;


	// helper function--operator<<
	template < typename StorageT, typename FilterT, typename T >
	inline log_t<StorageT, FilterT> &operator<<(log_t<StorageT, FilterT> &log, const T &val)
	{
		log.print(val);

		return log;
	}


	template < typename StorageT, typename FilterT, typename ValueT >
	inline log_t<StorageT, FilterT> &operator<<(log_t<StorageT, FilterT> &log, const format::function_obj_t<ValueT> &obj)
	{
		obj(log);

		return log;
	}
	
}






#endif
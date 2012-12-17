#ifndef TUT_EXCEPTION_H_GUARD
#define TUT_EXCEPTION_H_GUARD

#include <stdexcept>
#include "tut_result.hpp"

namespace tut
{

	namespace detail
	{
		template < typename >
		struct select;

		template <>
		struct select<char>
		{
			std::basic_string<char> val_;

			select(const char *val, const wchar_t *)
				: val_(val)
			{}
			
			operator const std::basic_string<char> &() const
			{
				return val_;
			}
		};

		template <>
		struct select<wchar_t>
		{
			std::basic_string<wchar_t> val_;

			select(const char *, const wchar_t *val)
				: val_(val)
			{}

			operator const std::basic_string<wchar_t> &() const
			{
				return val_;
			}
		};
	}

	/**
	* The base for all TUT exceptions.
	*/

	template < typename CharT > 
	struct tut_error_t 
		: public std::exception
	{
		explicit tut_error_t(const std::basic_string<CharT>& msg)
			: err_msg(msg)
		{
		}

		virtual test_result::result_type result() const
		{
			return test_result::ex;
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::tut_error", L"tut::tut_error");
		}

		const std::basic_string<CharT> &what_() const
		{
			return err_msg;
		}

		~tut_error_t() throw()
		{
		}

	private:
		void operator=(const tut_error_t &);

		const std::basic_string<CharT> err_msg;
	};

	template < typename CharT >
	tut_error_t<CharT> tut_error(const std::basic_string<CharT> &str)
	{
		return tut_error_t<CharT>(str);
	}

	template < typename CharT >
	tut_error_t<CharT> tut_error(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return tut_error_t<CharT>(val);
	}

	/**
	* Group not found exception.
	*/
	template < typename CharT >
	struct no_such_group_t 
		: public tut_error_t<CharT>
	{
		explicit no_such_group_t(const std::basic_string<CharT>& grp)
			: tut_error_t(grp)
		{
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::no_such_group", L"tut::no_such_group");
		}

		~no_such_group_t() throw()
		{
		}
	};

	template < typename CharT >
	no_such_group_t<CharT> no_such_group(const std::basic_string<CharT> &str)
	{
		return no_such_group_t<CharT>(str);
	}

	template < typename CharT >
	no_such_group_t<CharT> no_such_group(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return no_such_group_t<CharT>(val);
	}


	/**
	* Test not found exception.
	*/
	template < typename CharT >
	struct no_such_test_t 
		: public tut_error_t<CharT>
	{
		explicit no_such_test_t(const std::basic_string<CharT>& grp)
			: tut_error_t(grp)
		{
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::no_such_test", L"tut::no_such_test");
		}

		~no_such_test_t() throw()
		{
		}
	};

	template < typename CharT >
	no_such_test_t<CharT> no_such_test(const std::basic_string<CharT> &str)
	{
		return no_such_test_t<CharT>(str);
	}

	template < typename CharT >
	no_such_test_t<CharT> no_such_test(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return no_such_test_t<CharT>(val);
	}

	/**
	* Internal exception to be throwed when
	* test constructor has failed.
	*/
	template < typename CharT >
	struct bad_ctor_t 
		: public tut_error_t<CharT>
	{
		explicit bad_ctor_t(const std::basic_string<CharT>& msg)
			: tut_error_t(msg)
		{
		}

		test_result::result_type result() const
		{
			return test_result::ex_ctor;
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::bad_ctor", L"tut::bad_ctor");
		}

		~bad_ctor_t() throw()
		{
		}
	};

	template < typename CharT >
	bad_ctor_t<CharT> bad_ctor(const std::basic_string<CharT> &str)
	{
		return bad_ctor_t<CharT>(str);
	}

	template < typename CharT >
	bad_ctor_t<CharT> bad_ctor(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return bad_ctor_t<CharT>(val);
	}

	/**
	* Exception to be throwed when ensure() fails or fail() called.
	*/
	template < typename CharT >
	struct failure_t 
		: public tut_error_t<CharT>
	{
		explicit failure_t(const std::basic_string<CharT>& msg)
			: tut_error_t(msg)
		{
		}

		test_result::result_type result() const
		{
			return test_result::fail;
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::failure", L"tut::failure");
		}

		~failure_t() throw()
		{
		}
	};

	template < typename CharT >
	failure_t<CharT> failure(const std::basic_string<CharT> &str)
	{
		return failure_t<CharT>(str);
	}

	template < typename CharT >
	failure_t<CharT> failure(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return failure_t<CharT>(val);
	}

	/**
	* Exception to be throwed when test desctructor throwed an exception.
	*/
	template < typename CharT >
	struct warning_t 
		: public tut_error_t<CharT>
	{
		explicit warning_t(const std::basic_string<CharT>& msg)
			: tut_error_t(msg)
		{
		}

		test_result::result_type result() const
		{
			return test_result::warn;
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::warning", L"tut::warning");
		}

		~warning_t() throw()
		{
		}
	};

	template < typename CharT >
	warning_t<CharT> warning(const std::basic_string<CharT> &str)
	{
		return warning_t<CharT>(str);
	}

	template < typename CharT >
	warning_t<CharT> warning(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return warning_t<CharT>(val);
	}

	/**
	* Exception to be throwed when test issued SEH (Win32)
	*/
	template < typename CharT >
	struct seh_t 
		: public tut_error_t<CharT>
	{
		explicit seh_t(const std::basic_string<CharT>& msg)
			: tut_error_t(msg)
		{
		}

		virtual test_result::result_type result() const
		{
			return test_result::term;
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::seh", L"tut::seh");
		}

		~seh_t() throw()
		{
		}
	};

	template < typename CharT >
	seh_t<CharT> seh(const std::basic_string<CharT> &str)
	{
		return seh_t<CharT>(str);
	}

	template < typename CharT >
	seh_t<CharT> seh(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return seh_t<CharT>(val);
	}


	/**
	* Exception to be throwed when child processes fail.
	*/
	template < typename CharT >
	struct rethrown_t 
		: public failure_t<CharT>
	{
		explicit rethrown_t(const test_result &result)
			: failure_t(result.message), tr(result)
		{
		}

		virtual test_result::result_type result() const
		{
			return test_result::rethrown;
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::rethrown", L"tut::rethrown");
		}

		~rethrown_t() throw()
		{
		}

		const test_result tr;
	};

	template < typename CharT >
	rethrown_t<CharT> rethrown(const std::basic_string<CharT> &str)
	{
		return rethrown_t<CharT>(str);
	}

	template < typename CharT >
	rethrown_t<CharT> rethrown(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return rethrown_t<CharT>(val);
	}



	template < typename CharT >
	struct skipped_t 
		: public tut_error_t<CharT>
	{
		explicit skipped_t(const std::basic_string<CharT>& msg)
			: tut_error_t(msg)
		{
		}

		virtual test_result::result_type result() const
		{
			return test_result::skipped;
		}

		virtual std::basic_string<CharT> type() const
		{
			return detail::select<CharT>("tut::skipped", L"tut::skipped");
		}

		~skipped_t() throw()
		{
		}
	};

	template < typename CharT >
	skipped_t<CharT> skipped(const std::basic_string<CharT> &str)
	{
		return skipped_t<CharT>(str);
	}

	template < typename CharT >
	skipped_t<CharT> skipped(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return skipped_t<CharT>(val);
	}
}

#endif

#ifndef __LOG_LOG_HANDLER_HPP
#define __LOG_LOG_HANDLER_HPP



namespace log_system
{

	namespace format
	{
		// function Ends
		template < typename StreamT >
		inline StreamT &ends(StreamT &log)
		{
			log.NewLine();

			return log;
		}

		// function Endl
		template < typename StreamT >
		inline StreamT &endl(StreamT &log)
		{
			log.new_line();
			log.flush();

			return log;
		}

		// function Space
		template < typename StreamT >
		inline StreamT &space(StreamT &log)
		{
			log.whitespace();

			return log;
		}





		template < typename ValueT >
		class function_obj_t
		{
			typedef void (* function_ptr_type)(detail::basic_log &, const ValueT &, bool);

		private:
			function_ptr_type func_;
			ValueT val_;
			bool show_;

		public:
			function_obj_t(function_ptr_type func, const ValueT &val, bool show)
				: func_(func)
				, val_(val)
				, show_(show)
			{}

			void operator()(detail::basic_log &log) const
			{
				return (*func_)(log, val_, show_);
			}
		};


		// ----------------------------------------------------
		// Level

		template < typename LogT, typename ValueT >
		inline void level_helper(LogT &log, const ValueT &level, bool show)
		{
			log.priority(level, show);
		}


		template<typename ValueT>
		inline function_obj_t<ValueT> level(const ValueT &val, bool show = true)
		{
			return function_obj_t<ValueT>(&level_helper, val, show);
		}


		// ----------------------------------------------------
		// Color

		enum console_clr
		{ 
			Red = FOREGROUND_RED | FOREGROUND_INTENSITY, 
			Green = FOREGROUND_GREEN | FOREGROUND_INTENSITY, 
			Blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY
		};

		template < typename StreamT >
		inline void clr_helper(StreamT &/*stream*/, const console_clr &color, bool/* show*/)
		{
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), color);
		}

		inline function_obj_t<console_clr> clr(console_clr color)
		{
			return function_obj_t<console_clr>(&clr_helper, color, false);
		}
	}

}




#endif
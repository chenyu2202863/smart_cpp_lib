#ifndef __WX_LOG_HPP
#define __WX_LOG_HPP

#include "extend_stl/unicode.hpp"


namespace wx
{

	enum
	{
		Critical = 0x00,
		Error,
		Warning,
		Notice,
		Info,
		Debug,
		Trace,
		All
	};

	

	class log
	{
		struct impl;
		std::auto_ptr<impl> impl_;

		friend log &endl(log &);

	public:
		log(const stdex::tString &diretcory, const stdex::tString &name, int expired_days);
		~log();

	private:
		log(const log &);
		log &operator=(const log &);

	public:
		log &operator()(size_t level);

		void del_expired();

	public:
		log &operator<<(const char *val);
		log &operator<<(const std::string &val);
		log &operator<<(char val);
		log &operator<<(int val);
		log &operator<<(long val);
		log &operator<<(double val);
		log &operator<<(long long val);

		log &operator<<(log& (__cdecl *)(log&));
	};

	
	log &endl(log &);

}





#endif
#include "log.hpp"
#include "log/log.hpp"
#include "win32/file_system/file_helper.hpp"
#include "win32/file_system/filesystem.hpp"
#include "extend_stl/string/algorithm.hpp"
#include "unicode/string.hpp"

#include <atltime.h>

namespace wx
{
	struct log::impl
	{
		typedef log_system::storage::async_storage_t<log_system::storage::file_storage_t<char>> async_storage;
		typedef log_system::log_t<async_storage, log_system::filter::default_filter>			log_type;
		
		stdex::tString directory_;
		stdex::tString full_path_;
		int days_;

		log_type log_;
		
		impl(const stdex::tString &diretcory, const stdex::tString &name, int expired_days)
			: directory_(diretcory)
			, full_path_(win32::filesystem::combine_file_path(diretcory, name))
			, days_(expired_days)
			, log_(unicode::to_a(full_path_).c_str())
		{
			log_.priority(Info, true);
		}


		void del_log()
		{
			assert(win32::filesystem::is_directory_exist(directory_));
			
			win32::filesystem::depth_search(directory_,
				[this](const win32::filesystem::find_file &file)->bool
			{
				stdex::tString file_name = file.get_file_name();
				int year = stdex::to_number(file_name.substr(0, 4));
				int mon = stdex::to_number(file_name.substr(5, 2));
				int day = stdex::to_number(file_name.substr(8, 2));

				CTime start_time(year, mon, day, 0, 0, 0);
				CTime end_time = CTime::GetCurrentTime();

				if( (end_time - start_time).GetTimeSpan() > days_ * 24 * 60 * 60)
					return true;
				else
					return false;
			},
				[](const stdex::tString &path)
			{
				typedef utility::selector_t<stdex::tString::value_type, win32::file::path_traits, win32::file::wpath_traits>::type path_traits;
				typedef win32::file::basic_path<stdex::tString, path_traits> path_type;
				bool suc = win32::file::remove(path_type(path));
				assert(suc);
			});
		}
	};

	log::log(const stdex::tString &diretcory, const stdex::tString &name, int expired_time)
	{
		bool suc = win32::filesystem::mk_directory(diretcory);
		assert(suc);

		impl_.reset(new impl(diretcory, name, expired_time));
	}

	log::~log()
	{

	}

	log &log::operator()(size_t level)
	{
		impl_->log_ << '[' 
			<< unicode::to_a((LPCTSTR)CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S"))) 
			<< "  " << log_system::level(level) << "]: ";
		return *this;
	}

	log &log::operator<<(const char *val)
	{
		impl_->log_ << val;
		return *this;
	}

	log &log::operator<<(const std::string &val)
	{
		impl_->log_ << val;
		return *this;
	}

	log &log::operator<<(char val)
	{
		impl_->log_ << val;
		return *this;
	}

	log &log::operator<<(int val)
	{
		impl_->log_ << val;
		return *this;
	}

	log &log::operator<<(long val)
	{
		impl_->log_ << val;
		return *this;
	}

	log &log::operator<<(double val)
	{
		impl_->log_ << val;
		return *this;
	}

	log &log::operator<<(long long val)
	{
		impl_->log_ << val;
		return *this;
	}

	log &log::operator<<(log& (__cdecl *func)(log &))
	{
		func(*this);
		return *this;
	}

	void log::del_expired()
	{
		impl_->del_log();
	}


	log &endl(log &l)
	{
		l.impl_->log_.new_line();
		l.impl_->log_.flush();
		return l;
	}
	


}
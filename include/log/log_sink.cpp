#include "log_sink.hpp"
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <memory>
#include <system_error>
#include <cstdint>
#include <filesystem>

#include "../utility/utility.hpp"
#include "../unicode/string.hpp"

#include "../extend_stl/container/blocking_queue.hpp"
#include "../extend_stl/time.hpp"


namespace logging {

	buffer_t default_buffer_allocate(std::uint32_t size)
	{
		return std::make_pair(reinterpret_cast<char *>(::operator new(size)), size);
	}

	void default_buffer_dallocate(buffer_t p)
	{
		::operator delete(p.first);
	}

	struct async_sink_t::impl
	{
		std::fstream file_;
		log_callback_t log_callback_;

		buffer_allocate_t buffer_allocate_;
		buffer_deallocate_t buffer_deallocate_;

		impl(const std::string &file_name, const log_callback_t &callback)
			: log_callback_(callback)
		{
			std::string date_time = stdex::time::time_2_string<char>(stdex::time::now(), "%Y-%m-%d_%H%M%S");
			std::string path = unicode::to_a(utility::get_app_path()) + "log/" + date_time + "/";
			
			std::tr2::sys::create_directories(std::tr2::sys::path(path));

			std::ostringstream os;
			os << path << file_name
				<< '.'
				<< "log";

			file_.open(os.str().c_str(), std::fstream::out | std::fstream::binary);
			file_.sync_with_stdio(false);

			if( !file_.good() )
				throw std::system_error(std::make_error_code(std::errc::bad_file_descriptor));

			buffer_allocate_ = default_buffer_allocate;
			buffer_deallocate_ = default_buffer_dallocate;
		}

		~impl()
		{
			stop();
			file_.close();
		}

		void stop()
		{
			file_.flush();
			
		}

		void put(const buffer_t &msg)
		{
			log_callback_([msg, this]() 
			{
				if( msg.second == 0 )
					return;

				file_.write(msg.first, msg.second);
				buffer_deallocate_(msg);
			});
		}
	};

	async_sink_t::async_sink_t(const std::string &file_name, const log_callback_t &callback)
		: impl_(std::make_unique<impl>(file_name, callback))
	{

	}

	async_sink_t::~async_sink_t()
	{
	
	}

	void async_sink_t::stop()
	{
		impl_->stop();
	}

	void async_sink_t::set_buffer_allocator(const buffer_allocate_t &buffer_allocate, const buffer_deallocate_t &buffer_deallocate)
	{
		impl_->buffer_allocate_ = buffer_allocate;
		impl_->buffer_deallocate_ = buffer_deallocate;
	}

	void async_sink_t::log(const char *msg, std::uint32_t len)
	{
		buffer_t buffer = impl_->buffer_allocate_(len);
		std::copy(msg, msg + len, stdext::make_unchecked_array_iterator(buffer.first));

		impl_->put(buffer);
	}

	void async_sink_t::flush()
	{
		impl_->file_.flush();
	}
}
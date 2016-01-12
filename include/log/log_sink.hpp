#ifndef __LOG_SINK_HPP
#define __LOG_SINK_HPP

#include <string>
#include <memory>
#include <cstdint>
#include <functional>

namespace logging {

	typedef std::pair<char *, std::uint32_t> buffer_t;

	typedef std::function<buffer_t(std::uint32_t)>	buffer_allocate_t;
	typedef std::function<void(buffer_t)>			buffer_deallocate_t;


	class async_sink_t
	{
		typedef std::function<void(std::function<void()> &&)> log_callback_t;

		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		async_sink_t(const std::string &file_name, const log_callback_t &);
		~async_sink_t();

	private:
		async_sink_t(const async_sink_t &);
		async_sink_t &operator=(const async_sink_t &);


	public:
		void stop();
		void set_buffer_allocator(const buffer_allocate_t &, const buffer_deallocate_t &);

		void log(const char *, std::uint32_t);
		void flush();
	};
}


#endif
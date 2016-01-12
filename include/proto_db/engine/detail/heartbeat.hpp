#ifndef __DB_PROTOBUFFER_HEARTBEAT_HPP
#define __DB_PROTOBUFFER_HEARTBEAT_HPP

#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

#include "../config.hpp"
#include "db.hpp"
#include "connection_pool.hpp"


namespace async { namespace proto { namespace detail {


	struct heartbeat_t
	{
		typedef std::function<void(const std::function<void(const std::shared_ptr<connection_info_t> &)>)>	connection_handler_t;
		
		error_handler_t error_handler_;

		std::unique_ptr<std::thread> timer_thread_;
		std::condition_variable timer_condition_;

		connection_handler_t connection_handler_;
		
		heartbeat_t(const error_handler_t &, const connection_handler_t &);

		bool start();
		bool stop();
	};


}}}

#endif
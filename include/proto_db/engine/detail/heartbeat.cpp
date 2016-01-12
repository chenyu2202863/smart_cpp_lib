#include "heartbeat.hpp"
#include <algorithm>


namespace async { namespace proto { namespace detail {

	heartbeat_t::heartbeat_t(const error_handler_t &error_handler, const connection_handler_t &connection_handler)
		: error_handler_(error_handler)
		, connection_handler_(connection_handler)
	{}

	bool heartbeat_t::start()
	{
		timer_thread_ = std::make_unique<std::thread>([this]()
		{
			const std::string sql = "select now()";
			std::mutex timer_mutex;

			while(1)
			{
				{
					std::unique_lock<std::mutex> mutex(timer_mutex);
					auto ret = timer_condition_.wait_for(mutex, std::chrono::minutes(5));
					if( ret == std::cv_status::no_timeout )
						break;
				}


				connection_handler_([this, &sql](const std::shared_ptr<connection_info_t> &con)
				{
					try
					{
						assert(con->first); 
						con->second->execute(sql);
					}
					catch( const std::exception &e )
					{
						con->first = false;
						error_handler_(e.what());
					}
				});

			}
		});

		return true;
	}

	bool heartbeat_t::stop()
	{
		timer_condition_.notify_one();

		if(timer_thread_)
			timer_thread_->join();

		return true;
	}
}}}
#include "engine.hpp"

#include <WinSock2.h>
#include <cstdint>
#include <algorithm>

#include "../../async_io/service/dispatcher.hpp"
#include "../../async_io/timer/timer.hpp"
#include "../../utility/move_wrapper.hpp"

#include "detail/db.hpp"
#include "detail/build_sql.hpp"
#include "detail/parse_recordset.hpp"
#include "detail/exception.hpp"
#include "detail/heartbeat.hpp"
#include "detail/connection_pool.hpp"


namespace async { namespace proto {

	
	struct bind_t
	{
		const std::string sql_;
	
		bind_t(const std::string &sql)
			: sql_(sql)
		{}
	};

	struct engine_t::impl
	{
		error_handler_t error_handler_;
	
		std::unique_ptr<service::io_dispatcher_t> io_;

		detail::heartbeat_t heartbeat_;
		std::string connection_cmd_;

		detail::connection_pool_t connections_;

		impl(const error_handler_t &handler)
			: error_handler_(handler)
			, heartbeat_(error_handler_, [this](const std::function<void(const std::shared_ptr<detail::connection_info_t> &)> &handler)
		{ 
			auto size = connections_.size();
			while( size > 0 )
			{
				--size;

				auto connector = connections_.raw_aciquire();
				handler(std::cref(connector));
				connections_.raw_release(std::move(connector));
			}
		})
			, connections_([this]()
		{ 
			detail::connection_ptr connection(new detail::connection_t(error_handler_), [](detail::connection_t *connection)
			{
				connection->stop();
				delete connection;
			});

			return std::make_shared<detail::connection_info_t>(connection->start(connection_cmd_), connection);
		})
		{

		}

		~impl()
		{

		}

		bool start(const std::string &connection_cmd)
		{
			connection_cmd_ = connection_cmd;

			std::uint32_t num = service::get_fit_thread_num();
			io_.reset(new service::io_dispatcher_t(error_handler_, num, 
				[this]()
			{
				::CoInitialize(0);
			}, 
				[this]()
			{
				::CoUninitialize();
			}));

			return heartbeat_.start();
		}

		bool stop()
		{
			heartbeat_.stop();
			if( io_ )
				io_->stop();

			return true;
		}

		bool operate_impl(const std::string &sql, const operate_handler_t &handler)
		{
			auto move_sql = utility::make_move_obj(sql);
			auto move_handler = utility::make_move_obj(handler);

			io_->post([move_sql, this, move_handler](const std::error_code &, std::uint32_t)
			{
				try
				{
					auto val = connections_.get();
					if( !val->first )
						throw std::runtime_error("connection invalid");

					auto rds_id = val->second->execute(move_sql);
					move_handler(true, rds_id);
				}
				catch(std::exception &e)
				{
					error_handler_(e.what());

					move_handler(false, 0);
				}
			});

			return true;
		}

		bool execute_impl(const std::string &sql, const execute_handler_t &handler)
		{
			auto move_sql = utility::make_move_obj(sql);
			auto move_handler = utility::make_move_obj(handler);

			io_->post([move_sql, this, move_handler](const std::error_code &, std::uint32_t)
			{
				try
				{
					auto val = connections_.get();
					if( !val->first )
						throw std::runtime_error("connection invalid");

					val->second->get_recordset(move_sql);
					move_handler(true);
				}
				catch(std::exception &e)
				{
					error_handler_(e.what());

					move_handler(false);
				}
			});

			return true;
		}

		bool select_impl(const std::string &sql, const select_handler_t &handler, const std::string &table_name)
		{
			auto move_sql = utility::make_move_obj(sql);
			auto move_handler = utility::make_move_obj(handler);
			auto move_table_name = utility::make_move_obj(table_name);

			io_->post([move_sql, this, move_handler, move_table_name](const std::error_code &, std::uint32_t)
			{
				try
				{
					auto val = connections_.get();
					if( !val->first )
						throw std::runtime_error("connection invalid");

					const detail::recordset_ptr &rds = val->second->get_recordset(move_sql);

					std::vector<msg_ptr> values;
					values.reserve(rds->get_records_count());
					for(; !rds->is_eof(); rds->next())
					{
						detail::parse_rds parse;
						auto msg_val = parse.parse(move_table_name, rds);

						values.push_back(msg_val);
					}

					move_handler(true, std::cref(values));
				}
				catch(std::exception &e)
				{
					error_handler_(e.what());
					move_handler(false, std::vector<msg_ptr>());
				}
			});

			return true;
		}

		bool prepare_impl(const std::string &sql, const bind_handler_t &handler)
		{
			auto move_sql = utility::make_move_obj(sql);
			auto move_handler = utility::make_move_obj(handler);

			io_->post([this, move_sql, move_handler](const std::error_code &, std::uint32_t)
			{
				try
				{
					auto binder = std::make_shared<bind_t>(move_sql);
					move_handler(binder);
				}
				catch( std::exception &e )
				{
					error_handler_(e.what());
					move_handler(bind_ptr());
				}
			});

			return true;
		}

		bool bind_param_impl(const bind_ptr &binder, std::vector<_variant_t> &&var, const operate_handler_t &handler)
		{
			auto move_handler = utility::make_move_obj(handler);
			auto move_var = utility::make_move_obj(var);

			io_->post([this, binder, move_var, move_handler](const std::error_code &, std::uint32_t)
			{
				try
				{
					detail::build_sql build;
					auto command = std::make_shared<detail::command_t>(binder->sql_);
					build.build_binder(command, move_var);


					auto val = connections_.get();
					if( !val->first )
						throw std::runtime_error("connection invalid");

					record_id_t rds_id = val->second->execute(command);
					move_handler(true, rds_id);
				}
				catch(std::exception &e)
				{
					error_handler_(e.what());
					move_handler(false, 0);
				}
			});

			return true;
		}

		bool bind_param_impl(const bind_ptr &binder, const std::string &table_name, std::vector<_variant_t> &&var, const select_handler_t &handler)
		{
			auto move_handler = utility::make_move_obj(handler);
			auto move_table_name = utility::make_move_obj(table_name);
			auto move_var = utility::make_move_obj(var);

			io_->post([this, binder, move_table_name, move_var, move_handler](const std::error_code &, std::uint32_t)
			{
				try
				{
					detail::build_sql build;
					auto command = std::make_shared<detail::command_t>(binder->sql_);
					build.build_binder(command, move_var);

					auto val = connections_.get();
					if( !val->first )
						throw std::runtime_error("connection invalid");

					auto rds = val->second->get_recordset(command);

					std::vector<msg_ptr> values;
					values.reserve(10/*rds->get_records_count()*/);
					for( ; !rds->is_eof(); rds->next() )
					{
						detail::parse_rds parse;
						auto msg_val = parse.parse(move_table_name, rds);

						values.push_back(msg_val);
					}

					move_handler(true, std::cref(values));
				}
				catch( std::exception &e )
				{
					error_handler_(e.what());
					move_handler(false, std::vector<msg_ptr>());
				}
			});

			return true;
		}

		bool update_batch_impl()
		{
			io_->post([this](const std::error_code &, std::uint32_t)
			{
				try
				{
					auto val = connections_.get();
					if( !val->first )
						throw std::runtime_error("connection invalid");

					val->second->update_batch();
				}
				catch(std::exception &e)
				{
					error_handler_(e.what());
				}
			});

			return true;
		}
	};

	engine_t::engine_t(const error_handler_t &handler)
		: impl_(new impl(handler))
	{

	}

	engine_t::~engine_t()
	{

	}

	bool engine_t::start(const std::string &connection_cmd)
	{
		return impl_->start(connection_cmd);
	}

	bool engine_t::stop()
	{
		return impl_->stop();
	}

	bool engine_t::sql_insert(const gpb::Message &msg, const operate_handler_t &handler)
	{
		detail::build_sql build;
		std::string sql = build.build_insert(msg);

		return impl_->operate_impl(sql, handler);
	}

	bool engine_t::sql_update(const gpb::Message &msg, const gpb::Message &where, const operate_handler_t &handler)
	{
		detail::build_sql build;
		std::string sql = build.build_update(msg, where);
		return impl_->operate_impl(sql, handler);
	}

	bool engine_t::sql_delete(const gpb::Message &where, const operate_handler_t &handler)
	{
		detail::build_sql build;
		std::string sql = build.build_delete(where);
		return impl_->operate_impl(sql, handler);
	}

	bool engine_t::sql_execute(const std::string &sql, const execute_handler_t &handler)
	{
		return impl_->execute_impl(sql, handler);
	}

	bool engine_t::sql_select(const std::string &sql, const std::string &table_name, const select_handler_t &handler)
	{
		return impl_->select_impl(sql, handler, table_name);
	}

	bool engine_t::_select_impl(const std::string &table_name, const std::string &columns, const std::string &where, const select_handler_t &handler)
	{
		detail::build_sql build;
		std::string sql = build.build_select(table_name, columns, where);

		return impl_->select_impl(sql, handler, table_name);
	}


	bool engine_t::prepare(const std::string &sql, const bind_handler_t &handler)
	{
		return impl_->prepare_impl(sql, handler);
	}

	bool engine_t::bind_param(const bind_ptr &binder, const gpb::Message &msg, const operate_handler_t &handler)
	{
		try
		{
			/*detail::build_sql build;
			binder->command_ = std::make_shared<detail::command_t>(binder->sql_);
			build.build_binder(binder->command_, msg);

			impl_->bind_param_impl(binder, handler);*/
			return true;
		}
		catch(const std::exception &e)
		{
			impl_->error_handler_(e.what());
			return false;
		}
	}

	bool engine_t::bind_param(const bind_ptr &binder, std::vector<_variant_t> &&var, const operate_handler_t &handler)
	{
		try
		{
			impl_->bind_param_impl(binder, std::move(var), handler);
			return true;
		}
		catch(const std::exception &e)
		{
			impl_->error_handler_(e.what());
			return false;
		}
	}

	bool engine_t::bind_param(const bind_ptr &binder, std::vector<_variant_t> &&var, const select_handler_t &handler, const std::string &table_name)
	{
		try
		{
			impl_->bind_param_impl(binder, table_name, std::move(var), handler);
			return true;
		}
		catch( const std::exception &e )
		{
			impl_->error_handler_(e.what());
			return false;
		}
	}

	bool engine_t::update_batch()
	{
		return impl_->update_batch_impl();
	}
}
}
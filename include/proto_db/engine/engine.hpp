#ifndef __DB_PROTOBUF_ENGINE_HPP
#define __DB_PROTOBUF_ENGINE_HPP

#include <type_traits>
#include <vector>

#include "config.hpp"
#include "helper.hpp"


class _variant_t;

namespace async { namespace proto { 

	struct bind_t;
	typedef std::shared_ptr<bind_t> bind_ptr;

	class engine_t
	{
	public:
		typedef std::uint32_t record_id_t;
		typedef std::function<void(bool, record_id_t)>					operate_handler_t;
		typedef std::function<void(bool)>								execute_handler_t;
		typedef std::function<void(bool, const std::vector<msg_ptr> &)> select_handler_t;
		typedef std::function<void(const bind_ptr &)>					bind_handler_t;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		engine_t(const error_handler_t &);
		~engine_t();

	private:
		engine_t(const engine_t &);
		engine_t &operator=(const engine_t &);

	public:
		bool start(const std::string &connection_cmd);
		bool stop();

	public:
		bool sql_insert(const gpb::Message &msg, const operate_handler_t &);
		bool sql_update(const gpb::Message &msg, const gpb::Message &where, const operate_handler_t &);
		bool sql_delete(const gpb::Message &where, const operate_handler_t &);
		
		bool sql_execute(const std::string &sql, const execute_handler_t &);

		bool sql_select(const std::string &sql, const std::string &table_name, const select_handler_t &);
		template < typename T, typename ColumnT >
		bool sql_select(const ColumnT &cols, const where_t &where, const select_handler_t &);

	public:
		bool prepare(const std::string &sql, const bind_handler_t &);
		bool bind_param(const bind_ptr &, const gpb::Message &msg, const operate_handler_t &);
		bool bind_param(const bind_ptr &, std::vector<_variant_t> &&, const operate_handler_t &);
		bool bind_param(const bind_ptr &, std::vector<_variant_t> &&, const select_handler_t &, const std::string &table_name);

		bool update_batch();
	private:
		bool _select_impl(const std::string &table_name, const std::string &columns, const std::string &where, const select_handler_t &);
	};	

	template < typename T, typename ColumnT >
	bool engine_t::sql_select(const ColumnT &cols, const where_t &where, const select_handler_t &handler)
	{
		static_assert(std::is_base_of<gpb::Message, T>::value, "T must be derived from google::protobuf::Message");
		bool suc = _select_impl(T().GetTypeName(), cols.to_string(), where.to_string(), 
			[handler](bool suc, std::vector<msg_ptr> &val)
		{
			if( !suc )
				handler(true, std::vector<T>());
			else
			{
				std::vector<T> values;
				values.reserve(val.size());
				std::for_each(val.begin(), val.end(), [&values](const msg_ptr &val)
				{
					values.emplace_back(std::move(*std::static_pointer_cast<T>(val)));
				});

				handler(true, values);
			}
		});

		return suc;
	}


}
}


#endif
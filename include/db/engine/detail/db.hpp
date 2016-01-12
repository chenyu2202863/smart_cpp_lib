#ifndef __DB_PROTOBUF_CONNECTION_HPP
#define __DB_PROTOBUF_CONNECTION_HPP

#include <memory>
#include <string>
#include <cstdint>

#include <comutil.h>

#include "../config.hpp"
#include "param_traits.hpp"

namespace async { namespace proto { namespace detail { 

	struct recordset_t;
	class connection_t;
	class command_t;
	class parameters_t;

	typedef std::shared_ptr<recordset_t>	recordset_ptr;
	typedef std::shared_ptr<parameters_t>	parameters_ptr;
	typedef std::shared_ptr<command_t>		command_ptr;
	typedef std::shared_ptr<connection_t>	connection_ptr;



	struct recordset_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		recordset_t();
		~recordset_t();

	private:
		recordset_t(const recordset_t &);
		recordset_t &operator=(const recordset_t &);

	public:
		bool is_empty() const;
		bool is_eof() const;
		bool next();

		void set_connection(const connection_t &);
		void execute();

		std::uint32_t get_records_count() const;
		std::uint32_t get_field_count() const;
		std::string get_field_name(std::uint32_t index) const;
		_variant_t get_field_value(const std::string &filed_name) const;
		_variant_t get_field_value(std::uint32_t index) const;
	};
	

	// -----------------------------

	class command_t
	{
		friend parameters_t;

		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		command_t(const std::string &sql);
		~command_t();

	private:
		command_t(const command_t &);
		command_t &operator=(const command_t &);

	public:
		void set_connection(const connection_t &);
		recordset_ptr get_recordset();
		void execute();

		void append(const std::string &name, const param_t &param)
		{
			_append_impl(name, std::get<0>(param), std::get<1>(param), std::get<2>(param));
		}

		void append(long index, const _variant_t &v);

	private:
		void _append_impl(const std::string &, const _variant_t &, DataTypeEnum, std::uint32_t);
	};
	

	// -----------------------------
	class connection_t
	{
		friend command_t;
		friend recordset_t;

		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		explicit connection_t(const error_handler_t &);
		~connection_t();

		connection_t(connection_t &&rhs);

	private:
		connection_t(const connection_t &);
		connection_t &operator=(const connection_t &);

	public:
		bool start(const std::string &connection_cmd);
		bool stop();

		std::uint32_t execute(const std::string &sql);
		std::uint32_t execute(const command_ptr &);
		recordset_ptr get_recordset(const std::string &sql);
		recordset_ptr get_recordset(const command_ptr &);

		void update_batch();
	};
	


	class parameters_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;

		friend command_t;

	public:
		parameters_t(const command_ptr &command);
		~parameters_t();

	private:
		parameters_t(const parameters_t &);
		parameters_t &operator=(const parameters_t &);
	};
	
}}}


#endif
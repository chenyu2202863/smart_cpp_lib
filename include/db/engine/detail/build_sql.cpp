#include "build_sql.hpp"

#include <cstdint>
#include <google/protobuf/message.h>

#include "../../../utility/singleton.hpp"
#include "../../../serialize/serialize.hpp"

#include "convertion_traits.hpp"
#include "param_traits.hpp"
#include "db.hpp"


namespace async { namespace proto { namespace detail {

	typedef serialize::o_text_serialize stream_t;

	std::string translate_field_val(const gpb::Message &msg, const gpb::FieldDescriptor *field_desc, const gpb::Reflection *ref)
	{
		const auto type = field_desc->type();
		return utility::singleton_t<proto_2_db_t>::instance()[type](msg, field_desc, ref);
	}

	param_t param_field_val(const gpb::Message &msg, const gpb::FieldDescriptor *field_desc, const gpb::Reflection *ref)
	{
		const auto type = field_desc->cpp_type();
		return utility::singleton_t<param_type_2_db_type_t>::instance()[type](msg, ref, field_desc);
	}



	std::string build_sql::build_insert(const gpb::Message &msg)
	{
		char buffer[4096] = {0};
		stream_t os(buffer);

		os << "insert into "
			<< msg.GetTypeName() << " "
			<< "(";

		const gpb::Descriptor *desc = msg.GetDescriptor();
		const gpb::Reflection *ref = msg.GetReflection();


		std::uint32_t field_cnt = desc->field_count();

		bool is_first = true;
		for(std::uint32_t i = 0; i != field_cnt; ++i)
		{
			if( !ref->HasField(msg, desc->field(i)) )
				continue;

			if( !is_first )
				os << ", ";

			is_first = false;

			auto field_desc = msg.GetDescriptor()->field(i);

			assert(field_desc->label() == gpb::FieldDescriptor::LABEL_OPTIONAL || 
				field_desc->label() == gpb::FieldDescriptor::LABEL_REQUIRED);

			const std::string &name = field_desc->name();
			os << name;
		}

		os << ") values (";

		is_first = true;
		for(std::uint32_t i = 0; i != field_cnt; ++i)
		{
			if( !ref->HasField(msg, desc->field(i)) )
				continue;

			if( !is_first )
				os << ", ";

			is_first = false;

			auto field_desc = msg.GetDescriptor()->field(i);
			os << translate_field_val(msg, field_desc, ref);
		}

		os << ")";

		return std::string(buffer, os.in_length());
	}

	std::string build_sql::build_update(const gpb::Message &msg, const gpb::Message &where)
	{
		char buffer[4096] = { 0 };
		stream_t os(buffer);

		os << "update "
			<< msg.GetTypeName() << " "
			<< "set ";

		const gpb::Descriptor *desc = msg.GetDescriptor();
		const gpb::Reflection *ref = msg.GetReflection();

		std::uint32_t field_cnt = desc->field_count();

		bool is_first = true;
		for(std::uint32_t i = 0; i != field_cnt; ++i)
		{
			if( !ref->HasField(msg, desc->field(i)) )
				continue;

			if( !is_first )
				os << ", ";

			is_first = false;

			auto field_desc = desc->field(i);
			assert(field_desc->label() == gpb::FieldDescriptor::LABEL_OPTIONAL || 
				field_desc->label() == gpb::FieldDescriptor::LABEL_REQUIRED);

			const std::string &name = field_desc->name();
			os << name << " = " << translate_field_val(msg, field_desc, ref);
		}

		os << " where ";

		// where
		const gpb::Descriptor *where_desc = where.GetDescriptor();
		const gpb::Reflection *where_ref = where.GetReflection();

		std::uint32_t where_field_cnt = where_desc->field_count();
		bool is_where_first = true;
		for(std::uint32_t i = 0; i != where_field_cnt; ++i)
		{
			if( !ref->HasField(where, where_desc->field(i)) )
				continue;

			if( !is_where_first )
				os << " and ";

			is_where_first = false;

			auto field_desc = where.GetDescriptor()->field(i);
			const std::string &name = field_desc->name();
			os << name << " = " << translate_field_val(where, field_desc, where_ref);
		}

		return std::string(buffer, os.in_length());
	}

	std::string build_sql::build_delete(const gpb::Message &where)
	{
		char buffer[4096] = { 0 };
		stream_t os(buffer);

		os << "delete from "
			<< where.GetTypeName() << " "
			<< "where ";

		const gpb::Descriptor *where_desc = where.GetDescriptor();
		const gpb::Reflection *where_ref = where.GetReflection();

		std::uint32_t where_field_cnt = where_desc->field_count();
		bool is_where_first = true;
		for(std::uint32_t i = 0; i != where_field_cnt; ++i)
		{
			if( !where_ref->HasField(where, where_desc->field(i)) )
				continue;

			if( !is_where_first )
				os << " and ";

			is_where_first = false;

			auto field_desc = where_desc->field(i);
			assert(field_desc->label() == gpb::FieldDescriptor::LABEL_OPTIONAL || 
				field_desc->label() == gpb::FieldDescriptor::LABEL_REQUIRED);

			const std::string &name = field_desc->name();
			os << name << " = " << translate_field_val(where, field_desc, where_ref);
		}

		return std::string(buffer, os.in_length());
	}


	std::string build_sql::build_select(const std::string &table_name, const std::string &columns, const std::string &where)
	{
		char buffer[4096] = { 0 };
		stream_t os(buffer);

		os << "select " << columns << " from " << table_name << " " << where;

		return std::string(buffer, os.in_length());
	}

	bool build_sql::build_binder(const command_ptr &command, const gpb::Message &msg)
	{
		const gpb::Descriptor *desc = msg.GetDescriptor();
		const gpb::Reflection *reflect = msg.GetReflection();

		std::uint32_t field_cnt = desc->field_count();
		for(std::uint32_t i = 0; i != field_cnt; ++i)
		{
			if( !reflect->HasField(msg, desc->field(i)) )
				continue;

			auto field_desc = msg.GetDescriptor()->field(i);

			assert(field_desc->label() == gpb::FieldDescriptor::LABEL_OPTIONAL ||
				   field_desc->label() == gpb::FieldDescriptor::LABEL_REQUIRED);

			const std::string &name = field_desc->name();
			command->append(name, param_field_val(msg, field_desc, reflect));
		}

		return true;
	}

	bool build_sql::build_binder(const command_ptr &command, const std::vector<_variant_t> &var)
	{
		for(auto i = 0; i != var.size(); ++i)
		{
			command->append(i, var[i]);
		}

		return true;
	}
}}}
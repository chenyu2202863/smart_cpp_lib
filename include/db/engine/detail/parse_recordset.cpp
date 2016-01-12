#include "parse_recordset.hpp"

#include "../../../utility/singleton.hpp"
#include "convertion_traits.hpp"
#include "exception.hpp"


namespace async { namespace proto { namespace detail {


	msg_ptr create_message(const std::string &name)
	{
		msg_ptr msg;
		const gpb::Descriptor* descriptor = gpb::DescriptorPool::generated_pool()->FindMessageTypeByName(name);

		if( descriptor )
		{
			const gpb::Message* prototype = gpb::MessageFactory::generated_factory()->GetPrototype(descriptor);
			if( prototype )
			{
				msg.reset(prototype->New());
			}
		}

		return msg;
	}

	void translate_db_val(const _variant_t &val, gpb::Message *msg, const gpb::FieldDescriptor *field_desc, const gpb::Reflection *reflection)
	{
		utility::singleton_t<db_2_proto_t>::instance()[val.vt](val, msg, reflection, field_desc);
	}


	struct type_conversion_t
	{
		const msg_ptr &msg_;
		const detail::recordset_ptr &rds_;

		type_conversion_t(const msg_ptr &msg, const detail::recordset_ptr &rds)
			: msg_(msg)
			, rds_(rds)
		{}

		void run()
		{
			std::uint32_t filed_cnt = rds_->get_field_count();

			const gpb::Descriptor *desc = msg_->GetDescriptor();
			const gpb::Reflection *reflection = msg_->GetReflection();


			for(std::uint32_t i = 0; i != filed_cnt; ++i)
			{
				std::string filed_name = rds_->get_field_name(i);
				_variant_t filed_value = rds_->get_field_value(i);

				const gpb::FieldDescriptor *field_desc = desc->FindFieldByName(filed_name);
				if( !field_desc )
					continue;

				translate_db_val(filed_value, msg_.get(), field_desc, reflection);
			}
		}
	};


	msg_ptr parse_rds::parse(const std::string &table_name, const recordset_ptr &rds)
	{
		msg_ptr msg_val = create_message(table_name);
		
		assert(msg_val);
		if( !msg_val )
			throw proto_exception("msg create failed");

		type_conversion_t type_conversion_val(msg_val, rds);
		type_conversion_val.run();

		return msg_val;
	}
}}}
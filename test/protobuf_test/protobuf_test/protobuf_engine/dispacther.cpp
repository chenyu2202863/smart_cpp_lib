#include "dispatcher.hpp"

#include <unordered_map>

#include "extend_stl/container/sync_container.hpp"


namespace proto_engine
{
	struct dispatcher_t::impl
	{
		typedef std::shared_ptr<callback_base_t> callback_handler_type;
		typedef std::unordered_map<const google::protobuf::Descriptor*,  callback_handler_type> callback_handlers_type;
		typedef stdex::container::sync_assoc_container_t<typename callback_handlers_type::key_type, typename callback_handlers_type::mapped_type, callback_handlers_type> callback_container;

		callback_container callbacks_;
		protobuf_msg_callback default_handler_;

	public:
		impl(const protobuf_msg_callback& default_handler)
			: default_handler_(default_handler)
		{
		}

		void on_protobuf_msg(session_ptr& impl, const msg_ptr& message)
		{
			callback_container::mapped_type tmp;
			callbacks_.op_if(message->GetDescriptor(), [&tmp](const callback_container::value_type &val)
			{
				tmp = val.second;
			});

			if( tmp != 0 )
			{
				tmp->on_msg(impl, message);
			}
			else
			{
				default_handler_(std::ref(impl), std::cref(message));
			}
		}

		template < typename T >
		void register_callback(const typename callback_t<T>::msg_callback_type &handler)
		{
			std::shared_ptr<callback_base_t> pd(new callback_t<T>(handler));

			callbacks_.insert(T::descriptor(), pd);
		}

		template < typename T >
		void unregister_callback()
		{
			callbacks_.erase(T::descriptor());
		}

	};

	
	dispatcher_t::dispatcher_t(const protobuf_msg_callback& default_handler)
		: impl_(new impl(default_handler))
	{}

	dispatcher_t::~dispatcher_t()
	{

	}

	void dispatcher_t::on_protobuf_msg(session_ptr& impl, const msg_ptr& message)
	{
		impl_->on_protobuf_msg(impl, message);
	}

	void dispatcher_t::_insert(const google::protobuf::Descriptor *desc, const callback_base_ptr &callback)
	{
		impl_->callbacks_.insert(desc, callback);
	}

	void dispatcher_t::_erase(const google::protobuf::Descriptor *desc)
	{
		impl_->callbacks_.erase(desc);
	}
}
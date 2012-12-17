#ifndef __PROTOBUF_ENGINE_DISPATCHER_HPP
#define __PROTOBUF_ENGINE_DISPATCHER_HPP



#include <functional>
#include <memory>
#include <type_traits>

#include "base.hpp"
#include "utility/object_factory.hpp"


namespace proto_engine
{
		
	class session;
	typedef std::shared_ptr<session> session_ptr;

	class callback_base_t
	{
	public:
		virtual ~callback_base_t() {};
		virtual void on_msg(session_ptr&, const msg_ptr&) const = 0;
	};
	typedef std::shared_ptr<callback_base_t> callback_base_ptr;


	template < typename T >
	class callback_t 
		: public callback_base_t
		, public utility::new_delete_base_t<callback_t<T>>
	{
		GOOGLE_COMPILE_ASSERT((std::is_base_of<google::protobuf::Message, T>::value), N);

	public:
		typedef std::function<void (session_ptr &impl, const std::shared_ptr<T> &msg)> msg_callback_type;

	private:
		msg_callback_type handler_;

	public:
		explicit callback_t(const msg_callback_type& handler)
			: handler_(std::move(handler))
		{
		}

		virtual void on_msg(session_ptr& impl, const msg_ptr& message) const
		{
			std::shared_ptr<T> concrete = std::dynamic_pointer_cast<T>(message);
			assert(concrete != 0);

			handler_(std::ref(impl), std::cref(concrete));
		}
	};


	// ---------------------------

	class dispatcher_t
	{
		struct impl;
		std::auto_ptr<impl> impl_;

		typedef std::function<void (session_ptr&, const msg_ptr&)> protobuf_msg_callback;

	public:
		explicit dispatcher_t(const protobuf_msg_callback& default_handler);
		~dispatcher_t();

	public:
		void on_protobuf_msg(session_ptr& impl, const msg_ptr& message);

		template < typename T >
		void register_callback(const typename callback_t<T>::msg_callback_type &handler)
		{
			callback_base_ptr pd(new callback_t<T>(handler));

			_insert(T::descriptor(), pd);
		}

		template < typename T >
		void unregister_callback()
		{
			_erase(T::descriptor());
		}

	private:
		void _insert(const google::protobuf::Descriptor*, const callback_base_ptr &);
		void _erase(const google::protobuf::Descriptor*);
	};
}

#endif
#ifndef __PROTO_ENGINE_SERVER_ENGINE_HPP
#define __PROTO_ENGINE_SERVER_ENGINE_HPP

#include <cstdint>
#include "dispatcher.hpp"


namespace proto_engine
{

	// ----------------------

	class session;
	typedef std::shared_ptr<session> session_ptr;
	bool send(const session_ptr &remote, const google::protobuf::Message &msg);



	class server
	{
		typedef std::function<void(const session_ptr &, const std::string &msg)>	error_handler_type;
		typedef std::function<bool(const session_ptr &)>							accept_handler_type;
		typedef std::function<void(const session_ptr &)>							disconnect_handler_type;

	private:
		dispatcher_t dispatcher_;

		struct impl;
		std::auto_ptr<impl> impl_;

	public:
		server(std::uint16_t port, const error_handler_type &error_handler);
		~server();

	private:
		server(const server &);
		server &operator=(const server &);

	public:
		bool start();
		void stop();

		void register_session(const accept_handler_type &connect, const disconnect_handler_type &disconnect);

		template < typename MsgT >
		void register_callback(const std::function<void(const session_ptr &, const std::shared_ptr<MsgT> &)> &handler)
		{
			dispatcher_.register_callback<MsgT>(handler);
		}

		template < typename MsgT >
		void unregister_callback()
		{
			dispatcher_.unregister_callback<MsgT>();
		}
	};

}


#endif

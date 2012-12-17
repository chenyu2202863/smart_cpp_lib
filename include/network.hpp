#ifndef __WX_NETWORK_HPP
#define __WX_NETWORK_HPP

#include <cstdint>
#include <memory>
#include <functional>
#include <string>

namespace wx
{

	class session;
	typedef std::shared_ptr<session> session_ptr;


	typedef std::function<void(const session_ptr &, const std::string &msg)>	error_handler_type;
	typedef std::function<bool(const session_ptr &, const std::string &ip)>		accept_handler_type;
	typedef std::function<void(const session_ptr &)>							disconnect_handler_type;
	typedef std::function<void(const session_ptr &, std::uint32_t size)>		read_handler_type;
	typedef std::function<void(const session_ptr &, std::uint32_t size)>		write_handler_type;


	bool async_read(const session_ptr &remote, char *buf, size_t len);
	bool async_write(const session_ptr &remote, const char *buf, size_t len);
	void close(const session_ptr &remote);
	std::string get_ip(const session_ptr &remote);

	void register_read_handler(const session_ptr &remote, const read_handler_type &);
	void register_write_handler(const session_ptr &remote, const write_handler_type &);
	
	// ----------------------------------

	class server
	{
	
	private:	
		struct impl ;
		std::auto_ptr<impl> impl_;

	public:
		explicit server(std::uint16_t port);
		~server();

	private:
		server(const server &);
		server &operator=(const server &);

	public:
		bool start();
		bool stop();

		void register_error_handler(const error_handler_type &);
		void register_accept_handler(const accept_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);
	};


	// ----------------------------------

	struct io_thread
	{
		struct impl;
		std::auto_ptr<impl> impl_;

		io_thread();
		io_thread(size_t num);
		~io_thread();

	private:
		io_thread(const io_thread &);
		io_thread &operator=(const io_thread &);

	public:
		void stop();
	};

	class client
	{
	public:
		typedef size_t time_second_type;
		enum { DEFAULT_TIME_OUT = 3 };

	public:
		typedef std::function<void(const std::string &msg)>		error_handler_type;
		typedef std::function<void(bool)>						connect_handler_type;
		typedef std::function<void()>							disconnect_handler_type;
		typedef std::function<void(std::int32_t size)>			read_handler_type;
		typedef std::function<void(std::int32_t size)>			write_handler_type;

	private:	
		struct impl;
		std::shared_ptr<impl> impl_;

		
	public:
		client(io_thread &io);
		~client();

	private:
		client(const client &);
		client &operator=(const client &);

	public:
		bool start(const std::string &ip, std::uint16_t port, time_second_type time_out = 0);
		void stop();

		void register_error_handler(const error_handler_type &);
		void register_connect_handler(const connect_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);
		void register_read_handler(const read_handler_type &);
		void register_write_handler(const write_handler_type &);

	public:
		bool async_send(const char *buf, size_t len);
		bool async_recv(char *buf, size_t len);

		size_t send(const char *buf, size_t len);
		size_t recv(char *buf, size_t len);
	};
	
}

#endif
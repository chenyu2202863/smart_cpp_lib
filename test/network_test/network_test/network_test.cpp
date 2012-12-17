// network_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <unordered_map>

#include "network.hpp"
#include "extend_stl/container/sync_container.hpp"

//#pragma comment(lib, "network_d")


namespace svr
{
	struct remote_session
	{
		std::vector<char> buf_;
		wx::session_ptr session_;

		remote_session(const wx::session_ptr &session)
			: session_(session)
		{
			buf_.resize(8192);

			using namespace std::placeholders;
			wx::register_read_handler(session, std::bind(&remote_session::_handle_read, this, _1, _2));
			wx::register_write_handler(session, std::bind(&remote_session::_handle_write, this, _1, _2));
		}

		void async_read()
		{
			wx::async_read(session_, buf_.data(),buf_.size());
		}

		void async_write()
		{
			wx::async_write(session_, buf_.data(), buf_.size());
		}

		void _handle_read(const wx::session_ptr &session, std::uint32_t len)
		{
			assert(len == buf_.size());
 
			std::cout.write(buf_.data(), buf_.size());

			async_write();
		}

		void _handle_write(const wx::session_ptr &session, std::uint32_t len)
		{
			async_read();
		}
	};

	typedef std::shared_ptr<remote_session> remote_ptr;
	typedef stdex::container::sync_assoc_container_t<void *, remote_ptr, std::unordered_map<void *, remote_ptr>> remote_hash_table;



	void server_test()
	{
		wx::server server(5050);

		remote_hash_table remotes;

		// register
		server.register_error_handler([](const wx::session_ptr &val, const std::string &msg)
		{
			std::cout << msg;
			if( val )
				std::cout << "  (remote socket)" << std::endl;
		});


		server.register_accept_handler([&remotes](const wx::session_ptr &val, const std::string &address)->bool
		{
			//std::cout << "remote: " << address << std::endl;

			remote_ptr remote(new remote_session(val));
			remotes.insert(val.get(), remote);

			remote->async_read();

			return true;
		});

		server.register_disconnect_handler([&remotes](const wx::session_ptr &val)
		{
			remotes.erase(val.get());
		});


		server.start();

		system("pause");

		server.stop();

		system("pause");
	}

}

namespace cli
{
	struct client_session
	{
		wx::io_thread io_;
		wx::client client_;
		std::vector<char> buf_;

		int send_cnt_;
		int recv_cnt_;

		client_session()
			: client_(io_)
			, send_cnt_(0)
			, recv_cnt_(0)
		{
			buf_.resize(8192);

			client_.register_error_handler(std::bind(&client_session::error_handler, this, std::placeholders::_1));
			client_.register_connect_handler(std::bind(&client_session::connect_handler, this, std::placeholders::_1));
			client_.register_disconnect_handler(std::bind(&client_session::disconnct_handler, this));
			client_.register_read_handler(std::bind(&client_session::read_handler, this, std::placeholders::_1));
			client_.register_write_handler(std::bind(&client_session::write_handler, this, std::placeholders::_1));
		}

		void start()
		{
			bool suc = client_.start("127.0.0.1", 5050, 3);
			assert(suc);

			if( suc )
			{
				while(1)
				{
					client_.send(buf_.data(), buf_.size());
					client_.recv(&buf_[0], buf_.size());
				}	
				if( client_.send(buf_.data(), buf_.size()) )
					++send_cnt_;

			}
		}

		void stop()
		{
			//while( send_cnt_ != 0 || recv_cnt_ != 0 )
			//	;
			client_.stop();
		}

		void error_handler(const std::string &msg)
		{
			std::cout << msg << std::endl;
		}

		void connect_handler(bool suc)
		{
			if( !suc )
				return;

			std::cout << "connect success" << std::endl;
		}

		void disconnct_handler()
		{
			std::cout << "disconnect" << std::endl;

			stop();
		}

		void read_handler(size_t len)
		{
			--recv_cnt_;

			if( client_.send(buf_.data(), buf_.size()) )
				++send_cnt_;

			assert(len == buf_.size());
		}

		void write_handler(size_t len)
		{
			--send_cnt_;
			if( client_.recv(buf_.data(), buf_.size()) )
				++recv_cnt_;

			assert(len == buf_.size());
		}
	};

	void client_test()
	{
		client_session cl;
		cl.start();
		system("pause");
		cl.stop();
		system("pause");
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	if( argc < 2 )
		return -1;

	if( argv[1][0] == _T('s') )
		svr::server_test();
	if( argv[1][0] == _T('c') )
		cli::client_test();

	return 0;
}


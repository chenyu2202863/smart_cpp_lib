#ifndef __TIMER_SERVICE_HPP
#define __TIMER_SERVICE_HPP



#include "async_io/network/tcp.hpp"
#include "async_io/timer/timer.hpp"
#include "async_io/file/file.hpp"
#include "async_io/logger/logger.hpp"

#include "algorithm/crc32.hpp"
#include "extend_stl/string/algorithm.hpp"

using namespace async::iocp;
using namespace async::network;
using namespace async::timer;

volatile long g_ClientNum = 0;


class Session
	: public std::tr1::enable_shared_from_this<Session>
{
private:
	io_dispatcher &io_;
	tcp::socket socket_;
	async::logger::basic_log log_;
	std::vector<char> buf_;

	async::iocp::callback_type readCallback_;
	async::iocp::callback_type writeCallback_;

public:
	explicit Session(io_dispatcher &io, const socket_handle_ptr &sock)
		: io_(io)
		, socket_(sock)
		, log_(io, stdex::to_string(sock->native_handle()))
	{
		buf_.resize(1024 * 1024 + sizeof(size_t));
		socket_.io_control(non_blocking_io(true));
		::InterlockedIncrement(&g_ClientNum);

		log_.use_file(stdex::to_string(sock->native_handle()));
	}
	~Session()
	{
		//Stop();
		::InterlockedDecrement(&g_ClientNum);
	}


public:
	tcp::socket& GetSocket()
	{
		return socket_;
	}

	void Start()
	{
		try
		{		
			readCallback_	= std::tr1::bind(&Session::_HandleRead, shared_from_this(), _Size, _Error);
			writeCallback_	= std::tr1::bind(&Session::_HandleWrite, shared_from_this(), _Size, _Error);

			async_read(socket_, buffer(buf_), transfer_all(), readCallback_);

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			log_.log(e.what());
		}
	}

	void Stop()
	{
		socket_.close();

		readCallback_	= 0;
		writeCallback_	= 0;
	}

private:
	void _HandleRead(u_long bytes, u_long error)
	{
		try
		{
			if( bytes == 0 )
			{
				socket_.async_disconnect(std::tr1::bind(&Session::_DisConnect, shared_from_this()));
				return;
			}

			size_t crc = algorithm::crc::cac_crc32(&buf_[sizeof(size_t)], bytes - sizeof(size_t));
			size_t dst = *(size_t *)(&buf_[0]);
			assert(crc == dst);

			async::filesystem::file_handle_ptr file(async::filesystem::make_file(io_));
			file->open(_T("test.txt"), 
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				CREATE_ALWAYS, 
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED);
			
			async_write(*file, buffer(buf_), 0, transfer_all(), [file, this](u_long, u_long)
			{
				async_write(socket_, buffer(buf_), transfer_all(), writeCallback_);
			});

			
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			log_.log(e.what());
		}
	}

	void _HandleWrite(u_long bytes, u_long error)
	{
		try
		{		
			async_read(socket_, buffer(buf_), transfer_at_leat(1), readCallback_);
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			log_.log(e.what());
		}
	}

	void _DisConnect()
	{
		Stop();
	}
};

typedef std::tr1::shared_ptr<Session> SessionPtr;




// 定制自己的工厂
namespace async
{
	namespace iocp
	{
		template<>
		struct object_factory_t< Session >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(Session)>	PoolType;
			typedef object_pool_t< PoolType >								ObjectPoolType;
		};
	}
}


template<typename T>
struct NoneDeletor
{
	void operator()(T *)
	{}
};


inline SessionPtr CreateSession(io_dispatcher &io, const socket_handle_ptr &socket)
{
	return SessionPtr(object_allocate<Session>(io, socket), &object_deallocate<Session>);
	//return SessionPtr(new Session(io, socket));
}



class Server
{
private:
	io_dispatcher &io_;
	tcp::accpetor acceptor_;
	std::auto_ptr<timer_handle> timer_;
	async::logger::basic_log log_;

public:
	Server(io_dispatcher &io, short port)
		: io_(io)
		, acceptor_(io_, tcp::v4(), port, INADDR_ANY)
		, log_(io, "svr")
	{
		log_.use_file("svr");
	}

	~Server()
	{
		//_StopServer();
	}

public:
	void Start()
	{
		timer_.reset(new timer_handle(io_, 2000, 0, std::tr1::bind(&Server::_OnTimer, this)));
		timer_->async_wait();

		_StartAccept();
	}

	void Stop()
	{
		_StopServer();
	}

private:
	void _StartAccept()
	{		
		try
		{
			acceptor_.async_accept(0, 
				std::tr1::bind(&Server::_OnAccept, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2/*std::tr1::cref(acceptSock)*/));
		} 
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			log_.log(e.what());
		}
	}

	void _StopServer()
	{
		acceptor_.close();
	}

	void _OnTimer()
	{
		std::cout << "Clients: " << g_ClientNum << std::endl;

		//log_.log("clients");
	}

private:
	void _OnAccept(u_long error, const socket_handle_ptr &acceptSocket)
	{
		if( error != 0 )
			return;

		try
		{
			SessionPtr session(CreateSession(io_, acceptSocket));
			session->Start();
	
			_StartAccept();
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			log_.log(e.what());
		}
	}
};



#endif
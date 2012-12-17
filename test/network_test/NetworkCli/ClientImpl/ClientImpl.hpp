#ifndef __CLIENT_IMPL_HPP
#define __CLIENT_IMPL_HPP

#include <iostream>

#include "async_io/network/tcp.hpp"
#include "async_io/file/file.hpp"

#include "algorithm/crc32.hpp"

using namespace async::network;
using namespace async::iocp;


class Client
{
private:
	io_dispatcher &io_;
	tcp::socket socket_;
	std::vector<char> buf_;

public:
	Client(io_dispatcher &io, const std::string &ip, u_short port)
		: io_(io)
		, socket_(io, tcp::v4())
	{
		const size_t len = 1024 * 1024 + sizeof(size_t);
		buf_.resize(len);
		std::fill(buf_.begin(), buf_.end(), 100);

		size_t crc = algorithm::crc::cac_crc32(&buf_[sizeof(size_t)], len - sizeof(size_t));
		const char *beg = reinterpret_cast<char *>(&crc);
		const char *end = beg + sizeof(size_t);
		std::copy(beg, end, buf_.begin());
		

		try
		{
			//socket_.set
			socket_.async_connect(ip_address::parse(ip), port, 
				std::tr1::bind(&Client::_OnConnect, this, _Size, _Error));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}


private:
	void _OnConnect(u_long bytes, iocp::error_code error)
	{
		if( error != 0 )
			return;

		//static char msg[] = "I am a new client";

		try
		{
			async_write(socket_, buffer(buf_), transfer_all(), 
				std::tr1::bind(&Client::_HandleWrite, this, _Size, _Error));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _HandleRead(u_long bytes, u_long error)
	{
		try
		{
			if( bytes == 0 )
			{
				socket_.async_disconnect(std::tr1::bind(&Client::_DisConnect, this));
				return;
			}

			async::filesystem::file_handle_ptr file(async::filesystem::make_file(io_));
			file->open(_T("test.txt"), 
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				CREATE_ALWAYS, 
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED);

			::Sleep(2000);

			async_write(*file, buffer(buf_), 0, transfer_all(), [file, this](u_long, u_long)
			{
				async_write(socket_, buffer(buf_), transfer_all(), std::bind(&Client::_HandleWrite, this, _Size, _Error));
			});
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}

	}

	void _HandleWrite(u_long bytes, u_long error)
	{
		try
		{		
			if( bytes == 0 )
			{
				socket_.async_disconnect(std::tr1::bind(&Client::_DisConnect, this));
				return;
			}

			async_read(socket_, buffer(buf_), transfer_all(),
				std::tr1::bind(&Client::_HandleRead, this, _Size, _Error));

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _DisConnect()
	{
		socket_.close();
	}
};



#endif
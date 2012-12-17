// protobuf_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include "protobuf_engine/svr_engine.hpp"
#include "common.pb.h"


typedef std::shared_ptr<common_protocol::Response> response_ptr;

void response_handle(const proto_engine::session_ptr &remote, const response_ptr &response)
{
	//std::cout << response->err() << std::endl;
	proto_engine::send(remote, *response);
}


int g_val = 0;

void test_svr()
{
	proto_engine::server svr(5050, [](const proto_engine::session_ptr &remote, const std::string &msg)
	{
		std::cout << msg << std::endl;
	});

	svr.register_session([](const proto_engine::session_ptr &remote)->bool
	{
		
		return true; 
	}, 
		[](const proto_engine::session_ptr &remote)
	{
		//std::cout << g_val++ << std::endl;
		std::cout << "session disconnect" << std::endl; 
	});

	using namespace std::placeholders;

	std::function<void(const proto_engine::session_ptr &, const response_ptr &)> val = std::bind(&response_handle, _1, _2);
	svr.register_callback<response_ptr::element_type>(std::bind(&response_handle, _1, _2));

	svr.start();

	::system("pause");
	svr.stop();
}


int _tmain(int argc, _TCHAR* argv[])
{
	test_svr();
	

	return 0;
}


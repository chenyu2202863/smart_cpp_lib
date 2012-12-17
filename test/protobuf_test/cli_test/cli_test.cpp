// cli_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include "../protobuf_test/protobuf_engine/svr_engine.hpp"
#include "../protobuf_test/protobuf_engine/cli_engine.hpp"
#include "../protobuf_test/common.pb.h"

#include "multi_thread/thread.hpp"


typedef std::shared_ptr<common_protocol::Response> response_ptr;

void test_cli(const std::string &info)
{
	proto_engine::client cli([](const std::string &msg){ std::cout << msg << std::endl; });
	bool suc = cli.start("127.0.0.1", 5050);
	if( !suc )
		return;

	response_ptr::element_type val;
	val.set_ack(true);
	val.set_err(info);

	std::pair<bool, response_ptr> ret = cli.send<response_ptr::element_type>(val);
	if( !ret.first )
		std::cout << "发送失败" << std::endl;
	else
		std::cout << ret.second->err() << std::endl;

	::Sleep(2000);
	cli.stop();
}


int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<multi_thread::thread_impl_ex> threads(200);
	for(size_t i = 0; i != threads.size(); ++i)
	{
		threads[i].register_callback([i]()->DWORD
		{
			std::ostringstream os;
			os << i;
			for(size_t i = 0; i != 1000; ++i)
				test_cli(os.str());
			return 0;
		});
	}

	std::for_each(threads.begin(), threads.end(), [](multi_thread::thread_impl_ex &thr)
	{
		thr.start();
		//::Sleep(10);
	});

	std::for_each(threads.begin(), threads.end(), [](multi_thread::thread_impl_ex &thr)
	{
		thr.stop();
	});

	return 0;
}


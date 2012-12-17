// TestNetwork.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "impl/ServerImpl.h"
#include "async_io/timer/timer.hpp"

#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


void AsyncPrint()
{
	std::cout << " remote number: " << g_ClientNum << std::endl;
}

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	io_dispatcher ioService(get_fit_thread_num());

	try
	{
		Server server(ioService, 5050);
		server.Start();

		async::timer::timer_handle time(ioService);
		time.async_wait(std::tr1::bind(&AsyncPrint), 2000, 0);

		ioService.dispatch([](u_long, u_long){std::cout << "dispatch" << std::endl;});

		system("pause");
		server.Stop();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	system("pause");
	return 0;
}

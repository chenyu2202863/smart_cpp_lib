#ifndef __NETWORK_ACCEPT_HPP
#define __NETWORK_ACCEPT_HPP


#include <system_error>
#include <cstdint>
#include <functional>

#include "../basic.hpp"
#include "socket.hpp"

namespace async {
	namespace service
	{
		extern std::_Ph<2> _Socket;
		extern std::_Ph<3> _Address;
	}

	namespace network { namespace details {

		static const size_t SOCKET_ADDR_SIZE = sizeof(sockaddr_in) + 16;

		// Hook User Accept Callback
		template < typename HandlerT, typename BufferT >
		struct accept_handle_t
		{	
			socket_handle_t &acceptor_;
			socket_handle_t remote_sck_;
			BufferT buf_;
			HandlerT handler_;

			accept_handle_t(socket_handle_t &acceptor, socket_handle_t &&remoteSocket, const BufferT &buf, HandlerT &&handler)
				: acceptor_(acceptor)
				, remote_sck_(std::move(remoteSocket))
				, buf_(std::move(buf))
				, handler_(std::move(handler))
			{}

			accept_handle_t(accept_handle_t &&rhs)
				: acceptor_(rhs.acceptor_)
				, remote_sck_(std::move(rhs.remote_sck_))
				, buf_(std::move(rhs.buf_))
				, handler_(std::move(rhs.handler_))
			{

			}

			~accept_handle_t()
			{

			}

		private:
			accept_handle_t(const accept_handle_t &);
			accept_handle_t &operator=(const accept_handle_t &);

		public:
			void operator()(std::error_code error, std::uint32_t size)
			{
				// ∏¥÷∆Listen socket Ù–‘
				update_accept_context context(acceptor_);
				remote_sck_.set_option(context);

				sockaddr *local = 0, *remote = 0;
				int local_size = 0, remote_size = 0;

				socket_provider::singleton().GetAcceptExSockaddrs(reinterpret_cast<char *>(buf_.get()), 0, 
					SOCKET_ADDR_SIZE, SOCKET_ADDR_SIZE, 
					&local, &local_size, 
					&remote, &remote_size);

				handler_(std::cref(error), std::ref(remote_sck_), reinterpret_cast<SOCKADDR_IN *>(remote));
			}
		};
	}

	}

}



#endif
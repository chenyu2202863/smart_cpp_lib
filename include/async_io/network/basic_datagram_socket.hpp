#ifndef __NETWORK_DATAGRAM_HPP
#define __NETWORK_DATAGRAM_HPP




namespace async
{
	namespace network
	{

		// ---------------------------------------------------
		// class BasicDatagramSocket

		template<typename ProtocolT>
		class basic_datagram_socket_t
		{
		public:
			typedef ProtocolT						ProtocolType;
			typedef socket_handle_ptr						ImplementType;
			typedef socket_handle::dispatcher_type	AsyncIODispatcherType;	

		private:
			ImplementType impl_;

		public:
			explicit basic_datagram_socket_t(AsyncIODispatcherType &io)
				: impl_(make_socket(io))
			{}
			explicit basic_datagram_socket_t(const ImplementType &impl)
				: impl_(impl)
			{}

			basic_datagram_socket_t(AsyncIODispatcherType &io, const ProtocolType &protocol)
				: impl_(make_socket(io, protocol.Family(), protocol.Type(), protocol.Protocol()))
			{}
			basic_datagram_socket_t(AsyncIODispatcherType &io, const ProtocolType &protocol, u_short port)
				: impl_(make_socket(io, protocol.Family(), protocol.Type(), protocol.Protocol()))
			{
				impl_->bind(protocol.Family(), port, INADDR_ANY);
			}

		public:
			// 显示获取
			ImplementType &Get() 
			{
				return impl_;
			}
			const ImplementType &Get() const
			{
				return impl_;
			}

			// 支持隐式转换
			operator ImplementType()
			{
				return impl_;
			}
			operator const ImplementType() const
			{
				return impl_;
			}


		public:
			void Open(const ProtocolType &protocol = ProtocolType::V4())
			{
				impl_->open(protocol.Family(), protocol.Type(), protocol.Protocol());
			}

			bool IsOpen() const
			{
				return impl_->is_open();
			}

			void Close()
			{
				impl_->close();
			}

			void Cancel()
			{
				return impl_->cancel();
			}

			template<typename SetSocketOptionT>
			bool SetOption(const SetSocketOptionT &option)
			{
				return impl_->set_option(option);
			}
			template<typename GetSocketOptionT>
			bool GetOption(GetSocketOptionT &option)
			{
				return impl_->get_option(option)
			}
			template<typename IOControlCommandT>
			bool IOControl(IOControlCommandT &control)
			{
				return impl_->io_control(control);
			}

			void Bind(int family, u_short port, const ip_address &addr)
			{
				impl_->bind(family, port, addr);
			}


			// 连接远程服务
			void Connect(int family, const ip_address &addr, u_short port)
			{
				impl_->connect(family, addr, port);
			}

			void DisConnect(int shut = SD_BOTH)
			{
				impl_->dis_connect(shut, true);
			}

			// 异步链接
			template<typename HandlerT>
			async_callback *AsyncConnect(const ip_address &addr, u_short port, const HandlerT &handler)
			{
				return impl_->async_connect(addr, port, handler);
			}
			template<typename AsyncT>
			const AsyncT &AsyncConnect(const AsyncT &result, const ip_address &addr, u_short uPort)
			{
				return impl_->async_connect(result, addr, uPort);
			}

			
			// 阻塞式发送数据直到数据发送成功或出错
			template<typename ConstBufferT>
			size_t SendTo(const ConstBufferT &buffer, const SOCKADDR_IN *addr, u_long flag = 0)
			{
				return impl_->send_to(buffer, addr, flag);
			}

			// 阻塞式接收数据直到成功或出错
			template<typename MutableBufferT>
			size_t RecvFrom(MutableBufferT &buffer, SOCKADDR_IN *addr, u_long flag = 0)
			{
				return impl_->recv_from(buffer, addr, flag);
			}



			// 异步发送数据
			template<typename ConstBufferT, typename HandlerT>
			void AsyncSendTo(const ConstBufferT &buffer, const SOCKADDR_IN *addr, const HandlerT &callback)
			{
				return impl_->async_send_to(buffer, addr, callback);
			}

			template<typename MutableBufferT, typename HandlerT>
			void AsyncRecvFrom(MutableBufferT &buffer, SOCKADDR_IN *addr, const HandlerT &callback)
			{
				return impl_->async_recv_from(buffer, addr, callback);
			}

		};
	}
}




#endif
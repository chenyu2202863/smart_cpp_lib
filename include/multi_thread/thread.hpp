#ifndef __THREAD_THREAD_HPP
#define __THREAD_THREAD_HPP

/** @thread.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* 线程类
*/

#include <process.h>
#include <Windows.h>
#include <functional>
#include <cassert>


/*
	thread_impl_ex	普通线程
	
	msg_impl_ex		消息线程
	
*/

namespace multi_thread
{

	namespace detail
	{
		//-----------------------------------------------------
		// Thread

		template < bool t_bManaged >
		class thread_t
		{
		public:
			HANDLE thread_;       // Handle to thread
			DWORD thread_id_;     // Thread ID
			bool suspended_;      // Thread currently suspended?

		public:
			thread_t(HANDLE hThread = 0) 
				: thread_(hThread)
				, thread_id_(0)
				, suspended_(false)
			{
			}

			~thread_t()
			{
				if( t_bManaged ) 
					release();
			}

			// Operations

			bool create(LPTHREAD_START_ROUTINE pThreadProc, LPVOID pParam = 0, int iPriority = THREAD_PRIORITY_NORMAL)
			{
				assert(thread_==NULL);
				assert(pThreadProc);

#if defined(_MT) || defined(_DLL)
				thread_ = (HANDLE)::_beginthreadex(NULL, 0, (UINT (WINAPI*)(void*)) pThreadProc, pParam, CREATE_SUSPENDED, (UINT*) &thread_id_);
#else
				thread_ = ::CreateThread(NULL, 0, pThreadProc, pParam, CREATE_SUSPENDED, &thread_id_);
#endif // _MT

				if( thread_ == NULL ) 
					return false;

				if( iPriority != THREAD_PRIORITY_NORMAL ) 
				{
					if( !::SetThreadPriority(thread_, iPriority) ) 
					{
						assert(!"Couldn't set thread priority");
					}
				}

				return ::ResumeThread(thread_) != (DWORD) -1;
			}

			bool release()
			{
				if( thread_ == NULL ) 
					return true;
				if( ::CloseHandle(thread_) == FALSE ) 
					return false;

				thread_ = 0;
				thread_id_ = 0;

				return true;
			}

			void attach(HANDLE hThread)
			{
				assert(thread_==NULL);
				thread_ = hThread;
			}

			HANDLE detach()
			{
				HANDLE hThread = thread_;
				thread_ = NULL;

				return hThread;
			}

			bool set_priority(int iPriority) const
			{
				assert(thread_);

				return ::SetThreadPriority(thread_, iPriority) == TRUE;
			}

			int get_priority() const
			{
				assert(thread_);

				return ::GetThreadPriority(thread_);
			}

			bool suspend()
			{
				assert(thread_);

				if( suspended_ ) 
					return true;

				if( ::SuspendThread(thread_) == (DWORD) -1 ) 
					return false;

				suspended_ = true;

				return true;
			}

			bool resume()
			{
				assert(thread_);

				if( !suspended_ ) 
					return true;

				if( ::ResumeThread(thread_) == (DWORD) -1 ) 
					return false;

				suspended_ = false;

				return true;
			}

			bool is_suspended() const
			{
				assert(thread_);

				return suspended_ == true;
			}

			bool is_running() const
			{
				if( thread_ == 0 ) 
					return false;
				DWORD dwCode = 0;

				::GetExitCodeThread(thread_, &dwCode);

				return dwCode == STILL_ACTIVE;
			}

			bool wait_for_thread(DWORD dwTimeout = INFINITE) const
			{
				assert(thread_);

				return ::WaitForSingleObject(thread_, dwTimeout) == WAIT_OBJECT_0;
			}

			bool terminate(DWORD dwExitCode = 0) const
			{
				// See Q254956 why calling this could be a bad idea!
				assert(thread_);

				return TRUE == ::TerminateThread(thread_, dwExitCode);
			}

			DWORD get_thread_id() const
			{
				return thread_id_;
			}

			bool get_exit_code(DWORD* pExitCode) const
			{
				assert(thread_);
				assert(pExitCode);

				return ::GetExitCodeThread(thread_, pExitCode);
			}

#if(WINVER >= 0x0500)

			bool get_thread_times(LPFILETIME lpCreationTime, LPFILETIME lpExitTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime) const
			{
				assert(thread_);
				assert(lpExitTime != 0 && 
					lpKernelTime != 0 && 
					lpUserTime != 0);

				return ::GetThreadTimes(thread_, lpCreationTime, lpExitTime, lpKernelTime, lpUserTime);
			}

#endif // WINVER

#if(WINVER >= 0x0501)

			bool set_thread_affinity_mask(DWORD dwThreadMask)
			{
				assert(thread_);

				return ::SetThreadAffinityMask(thread_, dwThreadMask) != 0;
			}

			bool set_thread_ideal_processor(DWORD dwIdealProcessor)
			{
				assert(thread_);

				return ::SetThreadIdealProcessor(thread_, dwIdealProcessor) != (DWORD) -1;
			}

			DWORD get_thread_ideal_processor() const
			{
				assert(thread_);

				return ::SetThreadIdealProcessor(thread_, MAXIMUM_PROCESSORS);
			}

#endif // WINVER

			operator HANDLE() const 
			{ 
				return thread_; 
			}
		};

		typedef thread_t<false>	thread_handle_type;
		typedef thread_t<true>	thread_type;


		//----------------------------------------------------------------------
		// Thread Stop policy

		class stop_bool
		{
		public:
			volatile bool stopped_;
			stop_bool() 
				: stopped_(false) 
			{ };

			bool _clear_abort()        { stopped_ = false; return true; };
			bool _abort()             { stopped_ = true; return true; };
			bool _is_aborted() const   { return stopped_ == true; };
		};

		class stop_event
		{
		public:
			HANDLE stop_event_;

			stop_event()      
			{ stop_event_ = ::CreateEvent(NULL, TRUE, FALSE, NULL); };

			~stop_event()     
			{ ::CloseHandle(stop_event_); };

			bool _clear_abort()			{ return TRUE == ::ResetEvent(stop_event_); };
			bool _abort()				{ return TRUE == ::SetEvent(stop_event_); };
			bool _is_aborted() const	{ return ::WaitForSingleObject(stop_event_, 0) != WAIT_TIMEOUT; };
		};

		class stop_APC
		{
			static void CALLBACK APCProc(ULONG_PTR /*dwParam*/)
			{}

		public:
			bool _clear_abort()			{ return true; };
			bool _abort(HANDLE hThread)	{ ::QueueUserAPC(&APCProc, hThread, 0); return true; };
			bool _is_aborted() const		{ return false; };
		};

	}
	


	/**
	* @class <thread_implex_t>
	* @brief 线程类，提供所有线程操作相关接口
	*
	* TStopPolicy 结束策略，可以为stop_bool、stop_event、stop_APC
	* 继承thread_type,需要注册线程回调函数，类型为DWORD (*)(void)
	*/
	template < typename TStopPolicy >
	class thread_implex_t 
		: public detail::thread_type
		, private TStopPolicy
	{
		typedef std::function<DWORD(void)> callback_type;

	public:
		bool auto_delete_;     // Thread class will delete itself upon thread exit?
		bool auto_cleanup_;    // Thread class will wait for thread completion upon scope exit?

		callback_type handler_;	// Thread work function

	public:
		thread_implex_t() 
			: auto_delete_(false)
			, auto_cleanup_(true)
		{
		}

		/**
		* @brief 构造函数支持传输回调函数
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		thread_implex_t(const callback_type &handler)
			: auto_delete_(false)
			, auto_cleanup_(true)
			, handler_(handler)
		{

		}

		~thread_implex_t()
		{
			if( auto_cleanup_ ) 
				stop();
		}

	public:
		/**
		* @brief 注册线程回调函数
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		void register_callback(const callback_type &func)
		{
			handler_ = func;
		}

		/**
		* @brief 启动线程
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <启动线程之前必须先注册回调>
		* @remarks <无>
		*/
		bool start()
		{
			assert(handler_ != 0);

			if( !_clear_abort() )
				return false;
			if( !create(_thread_impl, reinterpret_cast<LPVOID>(this)) ) 
				return false;

			return true;
		}

		/**
		* @brief 启动线程
		* @param <dwTime> <超时等待时间，默认为INFINITE>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		void stop(DWORD dwTime = INFINITE)
		{
			if( !abort() ) 
				return;

			if( wait_for_thread(dwTime) )
				terminate((DWORD)-1);

			release();
		}

		/**
		* @brief 强制关闭线程
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <不推荐使用>
		* @remarks <无>
		*/
		bool abort()
		{
			if( thread_ == NULL ) 
				return false;
			if( !_abort() )
				return false;
			if( suspended_ ) 
				resume();

			return true;
		}

		/**
		* @brief 启动线程并等待线程执行结束
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		void join()
		{
			start();
			stop();
		}

		/**
		* @brief 返回线程是否设置结束标志
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		bool is_aborted() const
		{
			assert(thread_);

			return _is_aborted();
		}

		void set_auto_clean(bool bAutoClean = true)
		{
			auto_cleanup_ = bAutoClean;
		}

		void set_delete_on_exit(bool bAutoDelete = true)
		{
			auto_delete_ = bAutoDelete;
			auto_cleanup_ = !bAutoDelete;
		}

		// Static members

		static DWORD WINAPI _thread_impl(LPVOID pData)
		{
			thread_implex_t* pThis = reinterpret_cast<thread_implex_t *>(pData);

			// Not Register?
			if( pThis->handler_ == NULL )
				return (DWORD)-1;

#if defined(_MT) || defined(_DLL)
			_endthreadex(pThis->handler_());
			if( pThis->auto_delete_ ) 
				delete pThis;
			return 0;
#else
			DWORD dwRet = pThis->handler_();
			if( pThis->auto_delete_ ) 
				delete pThis;
			return dwRet;
#endif // _MT
		}
	};

	typedef thread_implex_t<detail::stop_bool>		thread_impl_ex;
	typedef thread_implex_t<detail::stop_event>		thread_impl_ex_event;



	/**
	* @class <msg_implex_t>
	* @brief 继承自thread_implex_t，并提消息队列
	*
	* TStopPolicy 结束策略，可以为stop_bool、stop_event、stop_APC
	* 需要注册线程回调函数，类型为void (*)(MSG &, LRESULT &)
	*/
	template < typename StopPolicyT >
	class msg_implex_t 
		: public thread_implex_t<StopPolicyT>
	{
		typedef std::function<void(MSG &, LRESULT &)> thread_msg_callback_type;

	public:
		thread_msg_callback_type msg_handler_;

	public:
		/**
		* @brief 注册回调函数
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		void register_callback(const thread_msg_callback_type &func)
		{
			msg_handler_ = func;
			handler_ = std::bind(&msg_implex_t::run, this);
		}

		/**
		* @brief 注册回调函数
		* @param <无>
		* @exception <不会抛出任何异常>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		bool post_quit_msg()
		{
			assert(thread_);
			if( thread_ == NULL ) 
				return FALSE;

			return 0 != ::PostThreadMessage(thread_id_, WM_QUIT, 0, 0L);
		}

		bool post_msg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L)
		{
			assert(thread_);
			if( thread_ == NULL ) 
				return FALSE;

			return 0 != ::PostThreadMessage(thread_id_, uMsg, wParam, lParam);
		}

		// Implementation

		DWORD run()
		{
			assert(msg_handler_ != NULL);
			// Not Register?
			if( msg_handler_ == NULL )
				return -1L;

			MSG msg;
			::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

			while( !is_aborted() )
			{
				LRESULT lResult = 0;
				msg_handler_(msg, lResult);
				if( lResult != S_OK )
					break;
			}

			return 0;
		}
	};

	typedef msg_implex_t<detail::stop_bool>		msg_thread_impl_ex;
	typedef msg_implex_t<detail::stop_event>	msg_thread_impl_ex_event;

}


#endif // __THREAD_THREAD_HPP


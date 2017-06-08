#pragma once
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 22
#endif
#else
#include <netinet/in.h>
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

struct SNetAddr
{
	char		szHost[INET_ADDRSTRLEN];
	uint16_t	nPort;

	SNetAddr(const char* szIP, uint16_t nPort)
		: nPort(nPort)
	{
		strncpy(this->szHost, szIP, INET_ADDRSTRLEN);
	}

	SNetAddr()
		: nPort(0)
	{
		strncpy(this->szHost, "0.0.0.0", INET_ADDRSTRLEN);
	}

	bool operator == (const SNetAddr& rhs) const
	{
		return strncmp(this->szHost, rhs.szHost, INET_ADDRSTRLEN) == 0 && this->nPort == rhs.nPort;
	}

	bool isAnyIP() const
	{
		if (memcmp(this->szHost, "0.0.0.0", strnlen("0.0.0.0", INET_ADDRSTRLEN - 1)) == 0)
			return true;

		return false;
	}
};

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace net
{

#ifdef _WIN32

#ifdef __BUILD_NETWORK_AS_DLL__
#ifdef __BUILD_NETWORK__
#	define __NETWORK_API__ __declspec(dllexport)
#else
#	define __NETWORK_API__ __declspec(dllimport)
#endif
#else
#ifdef __BUILD_NETWORK__
#	define __NETWORK_API__
#else
#	define __NETWORK_API__ extern
#endif
#endif

#else

#ifdef __BUILD_NETWORK__
#	define __NETWORK_API__
#else
#	define __NETWORK_API__ extern
#endif

#endif

	enum ENetConnecterState
	{
		eNCS_Connecting,		// 连接进程中
		eNCS_Connected,			// 连接完成
		eNCS_Disconnecting,		// 断开连接进行中
		eNCS_Disconnected,		// 断开连接完成

		eNCS_Unknown
	};

	enum ENetConnecterType
	{
		eNCT_Initiative,	// 主动连接
		eNCT_Passive,		// 被动连接

		eNCT_Unknown
	};

	enum ENetConnecterCloseType
	{
		eNCCT_Grace,	// 只是关闭本端发送，并且会保证已经在发送缓存中的数据发送完关闭，对端关闭由对端控制，本端会一直等待对端关闭
		eNCCT_Normal,	// 保证本端已经在发送缓存中的数据发送，然后关闭，即使没有收到对端关闭
		eNCCT_Force,	// 强制关闭，不等待发送缓存中国的数据被发送。
	};

	class INetConnecter;
	/**
	@brief: 网络连接处理器
	连接的超时机制需要逻辑层去处理，如果数据一直发送不了，或者连接已经是断开连接中，确迟迟无法全部断开，很可能是数据一直发送不了导致的，这个时候应该踢掉连接
	*/
	class INetConnecterHandler
	{
	protected:
		INetConnecter* m_pNetConnecter;	// 连接对象

	public:
		INetConnecterHandler() : m_pNetConnecter(nullptr) {}
		virtual ~INetConnecterHandler() {}

		/**
		@brief: 设置连接处理器
		*/
		void				setNetConnecter(INetConnecter* pConnecter) { this->m_pNetConnecter = pConnecter; }
		/**
		@brief: 获取连接处理器
		*/
		INetConnecter*		getNetConnecter() const { return this->m_pNetConnecter; }
		/**
		@brief: 发送数据，对于后端基于帧的服务器可以考虑cache的方式，对于网关服务器，上行包可以考虑cache，下行包直接发送比较好，并且网关服务器最好采用基于事件驱动的方式
		*/
		inline void			send(const void* pData, uint32_t nSize, bool bCache);
		/**
		@brief: 数据到达回调，这个pData必须在回调中消费掉，这个回调结束后pData的有效性不保证
		*/
		virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize) = 0;
		/**
		@brief: 数据发送完成回调（仅仅是发送到了socket缓存中，并不代表发送到对端，更别说被对方应用层处理，所以要准确的反馈数据被对方接收的事情需要应用层取处理）
		*/
		virtual void		onSendComplete(uint32_t nSize) = 0;
		/**
		@brief: 连接完成回调
		*/
		virtual void		onConnect() = 0;
		/**
		@brief: 连接断开回调
		*/
		virtual void		onDisconnect() = 0;
		/**
		@brief: 主动连接失败
		*/
		virtual void		onConnectFail() = 0;
	};

	class INetAccepter;
	/**
	@brief: 网络连接监听处理器
	*/
	class INetAccepterHandler
	{
	protected:
		INetAccepter* m_pNetAccepter;	// 监听对象

	public:
		INetAccepterHandler() : m_pNetAccepter(nullptr) {}
		virtual ~INetAccepterHandler() {}

		/**
		@brief: 设置监听处理器
		*/
		void                          setNetAccepter(INetAccepter* pAccepter) { this->m_pNetAccepter = pAccepter; }
		/**
		@brief: 获取监听处理器
		*/
		INetAccepter*                 getNetAccepter() const { return this->m_pNetAccepter; }
		/**
		@brief: 监听的端口有连接进来，试着获取对应连接的连接处理器
		*/
		virtual INetConnecterHandler* onAccept(INetConnecter* pNetConnecter) = 0;
	};

	/**
	@brief: 网络监听器接口
	*/
	class INetAccepter
	{
	public:
		INetAccepter() { }
		virtual ~INetAccepter() {}

		/**
		@brief: 设置监听器处理器
		*/
		virtual void            setHandler(INetAccepterHandler* pHandler) = 0;
		/**
		@brief: 获取监听地址
		*/
		virtual const SNetAddr& getListenAddr() const = 0;
		/**
		@brief: 关闭监听器
		*/
		virtual void            shutdown() = 0;
	};

	/**
	@brief: 网络连接器接口
	*/
	class INetConnecter
	{
	public:
		INetConnecter() { }
		virtual ~INetConnecter() { }

		/**
		@brief: 发送数据, 如果是缓存发送的模式，就直接把数据拷贝到网络层的缓存中，不然就是试着发送，发送不了才缓存
		*/
		virtual bool				send(const void* pData, uint32_t nSize, bool bCache) = 0;
		/**
		@brief: 关闭连接
		*/
		virtual void				shutdown(ENetConnecterCloseType eType, const char* szFormat, ...) = 0;
		/**
		@brief: 设置连接处理器
		*/
		virtual void				setHandler(INetConnecterHandler* pHandler) = 0;
		/**
		@brief: 获取连接本地地址
		*/
		virtual const SNetAddr&		getLocalAddr() const = 0;
		/**
		@brief: 获取连接远端地址
		*/
		virtual const SNetAddr&		getRemoteAddr() const = 0;
		/**
		@brief: 获取连接器类型
		*/
		virtual ENetConnecterType	getConnecterType() const = 0;
		/**
		@brief: 获取连接器状态
		*/
		virtual ENetConnecterState	getConnecterState() const = 0;
		/**
		@brief: 获取发送缓存区中的数据大小，这个一般缓存一帧数据，如果一帧内无法发送完数据还是会缓存
		*/
		virtual	uint32_t			getSendDataSize() const = 0;
		/**
		@brief: 获取接收缓存区大小
		*/
		virtual	uint32_t			getRecvDataSize() const = 0;
		/**
		@brief: 设置是否启动tcp协议栈的nodelay算法
		*/
		virtual bool				setNoDelay(bool bEnable) = 0;
	};

	/**
	@brief: 网络事件循环器
	*/
	class INetEventLoop
	{
	public:
		INetEventLoop() { }
		virtual ~INetEventLoop() {}

		/**
		@brief: 初始化
		*/
		virtual bool	init(uint32_t nMaxSocketCount) = 0;
		/**
		@brief: 发起一个监听
		*/
		virtual bool	listen(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetAccepterHandler* pHandler) = 0;
		/**
		@brief: 发起一个连接
		*/
		virtual bool	connect(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetConnecterHandler* pHandler) = 0;
		/**
		@brief: 推动网络事件循环器
		*/
		virtual void	update(int64_t nTime) = 0;
		/**
		@brief: 在事件循环等待中唤醒等待
		*/
		virtual void	wakeup() = 0;
		/**
		@brief: 释放网络事件循环器
		*/
		virtual void	release() = 0;
	};

	void INetConnecterHandler::send(const void* pData, uint32_t nSize, bool bCache)
	{
		if (this->m_pNetConnecter == nullptr)
			return;
		
		this->m_pNetConnecter->send(pData, nSize, bCache);
	}

	class INetworkLog
	{
	public:
		virtual ~INetworkLog() {}

		virtual void printInfo(const char* szFormat, ...) = 0;
		virtual void printWarning(const char* szFormat, ...) = 0;
	};

	__NETWORK_API__ bool			startupNetwork(INetworkLog* pNetworkLog);
	__NETWORK_API__ void			cleanupNetwork();

	__NETWORK_API__ INetEventLoop*	createNetEventLoop();
}
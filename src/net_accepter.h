#pragma once

#include "net_socket.h"
#include "network.h"

namespace net
{
	class CNetAccepter :
		public INetAccepter,
		public CNetSocket
	{
	public:
		CNetAccepter();
		virtual ~CNetAccepter();

		virtual void            onEvent(uint32_t nEvent);
		virtual void            forceClose();
		virtual bool			init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);

		virtual void            setHandler(INetAccepterHandler* pHandler);
		virtual const SNetAddr& getListenAddr() const;
		virtual void            shutdown();

		bool                    listen(const SNetAddr& netAddr);

	private:
		INetAccepterHandler*	m_pHandler;
#ifndef _WIN32
		int32_t					m_nIdleID;	// 这个主要为了解决服务器描述符用尽，这会导致accept()失败，并返还一个ENFILE 错误，但是并没有拒绝这个连接，连接仍然在连接队列中，这导致在下一次迭代的时候，仍然会触发监听描述符的可读事件，这导致程序busy loop
#endif
	};
}
#pragma once

#include "net_socket.h"
#include "network.h"

namespace base
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
		INetAccepterHandler* m_pHandler;
	};
}
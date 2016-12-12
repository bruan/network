#include "stdafx.h"
#include "net_event_loop.h"
#include "net_accepter.h"
#include "net_connecter.h"

namespace base
{
	void CNetAccepter::onEvent(uint32_t nEvent)
	{
		if (nEvent&eNET_Error)
			g_pLog->printInfo("CNetAccepter eNET_Error");

		if (eNET_Recv&nEvent)
		{
			for (uint32_t i = 0; i < 10; ++i)
			{
				int32_t nSocketID = (int32_t)::accept(this->GetSocketID(), nullptr, nullptr);

				if (_Invalid_SocketID == nSocketID)
				{
					if (getLastError() == NW_EINTR)
						continue;

					if (getLastError() != NW_EWOULDBLOCK
						&& getLastError() != NW_ECONNABORTED
						&& getLastError() != NW_EPROTO)
					{
						g_pLog->printWarning("NetAccepter::onEvent Error: %d", getLastError());
					}
					break;
				}

				if (this->m_pNetEventLoop->getSocketCount() >= this->m_pNetEventLoop->getMaxSocketCount())
				{
					g_pLog->printWarning("out of max connection[%d]", this->m_pNetEventLoop->getMaxSocketCount());
					closesocket(nSocketID);
					break;
				}

				g_pLog->printInfo("new connection accept listen addr: %s %d cur connection[%d]", this->getListenAddr().szHost, this->getListenAddr().nPort, this->m_pNetEventLoop->getSocketCount());

				CNetConnecter* pNetConnecter = new CNetConnecter();
				if (!pNetConnecter->init(this->getSendBufferSize(), this->getRecvBufferSize(), this->m_pNetEventLoop))
				{
					delete pNetConnecter;
					closesocket(nSocketID);
					continue;
				}
				pNetConnecter->setSocketID(nSocketID);
				pNetConnecter->setLocalAddr();
				pNetConnecter->setRemoteAddr();
				pNetConnecter->setConnecterType(eNCT_Passive);
				pNetConnecter->setConnecterState(eNCS_Connecting);
				if (!pNetConnecter->nonblock())
				{
					pNetConnecter->shutdown(true, "set non block error");
					continue;
				}
				INetConnecterHandler* pHandler = this->m_pHandler->onAccept(pNetConnecter);
				if (nullptr == pHandler)
				{
					pNetConnecter->shutdown(true, "create hander error");
					continue;
				}
				if (!this->m_pNetEventLoop->addSocket(pNetConnecter))
				{
					pNetConnecter->shutdown(true, "add socket error");
					continue;
				}
				pNetConnecter->setHandler(pHandler);

				// 模拟底层发一个可写事件
				pNetConnecter->onEvent(eNET_Send);
			}
		}
	}

	void CNetAccepter::forceClose()
	{
		CNetSocket::forceClose();
		delete this;
	}

	bool CNetAccepter::init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop)
	{
		return CNetSocket::init(nSendBufferSize, nRecvBufferSize, pNetEventLoop);
	}

	CNetAccepter::CNetAccepter()
		: m_pHandler(nullptr)
	{
	}

	CNetAccepter::~CNetAccepter()
	{
		DebugAst(this->GetSocketID() == _Invalid_SocketID);
	}

	bool CNetAccepter::listen(const SNetAddr& netAddr)
	{
		if (!this->open())
			return false;
		
		if (!this->reuseAddr())
		{
			this->forceClose();
			return false;
		}

		if (!this->nonblock())
		{
			this->forceClose();
			return false;
		}
		// 监听到的连接会继承缓冲区大小
		if (!this->setBufferSize())
		{
			this->forceClose();
			return false;
		}

		this->m_sLocalAddr = netAddr;
		struct sockaddr_in listenAddr;
		listenAddr.sin_family = AF_INET;
		// 不能用::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		listenAddr.sin_port = htons(this->m_sLocalAddr.nPort);
		listenAddr.sin_addr.s_addr = this->m_sLocalAddr.isAnyIP() ? htonl(INADDR_ANY) : inet_addr(this->m_sLocalAddr.szHost);
		memset(listenAddr.sin_zero, 0, sizeof(listenAddr.sin_zero));
		if (0 != ::bind(this->GetSocketID(), (sockaddr*)&listenAddr, sizeof(listenAddr)))
		{
			g_pLog->printWarning("bind socket to %s %d error %d", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, getLastError());
			this->forceClose();
			return false;
		}

		if (0 != ::listen(this->GetSocketID(), SOMAXCONN))
		{
			g_pLog->printWarning("listen socket to %s %d error %d", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, getLastError());
			this->forceClose();
			return false;
		}
		g_pLog->printInfo("start listen %s %d socket_id: %d ", this->m_sLocalAddr.szHost, this->m_sLocalAddr.nPort, this->GetSocketID());

		// 接收器采用水平触发的方式
#ifndef _WIN32
		this->m_nEvent &= ~EPOLLET;
#endif
		this->m_pNetEventLoop->addSocket(this);

		return true;
	}

	void CNetAccepter::setHandler(INetAccepterHandler* pHandler)
	{
		DebugAst(pHandler != nullptr);

		this->m_pHandler = pHandler;
		pHandler->setNetAccepter(this);
	}

	const SNetAddr& CNetAccepter::getListenAddr() const
	{
		return this->m_sLocalAddr;
	}

	void CNetAccepter::shutdown()
	{
		this->close(false);
	}
}
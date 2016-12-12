#include "stdafx.h"
#include "net_socket.h"
#include "net_event_loop.h"

namespace base
{
	CNetSocket::CNetSocket()
		: m_pNetEventLoop(nullptr)
		, m_nSocketID(_Invalid_SocketID)
		, m_nSocketIndex(_Invalid_SocketIndex)
		, m_nEvent(eNET_Recv | eNET_Send)
		, m_nSendBufferSize(0)
		, m_nRecvBufferSize(0)
		, m_bWaitClose(false)
	{
#ifndef _WIN32
		this->m_nEvent |= EPOLLET;
#endif
	}

	CNetSocket::~CNetSocket()
	{
	}

	bool CNetSocket::init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop)
	{
		DebugAstEx(pNetEventLoop != nullptr, false);

		this->m_nSendBufferSize = nSendBufferSize;
		this->m_nRecvBufferSize = nRecvBufferSize;
		this->m_pNetEventLoop = pNetEventLoop;

		return true;
	}

	void CNetSocket::forceClose()
	{
		if (this->m_nSocketIndex != _Invalid_SocketIndex)
			this->m_pNetEventLoop->delSocket(this);

		if (this->m_nSocketID != _Invalid_SocketID)
		{
			g_pLog->printInfo("closesocket socket_id %d", this->m_nSocketID);
			::closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
		}
	}

	void CNetSocket::setSocketIndex(int32_t nIndex)
	{
		this->m_nSocketIndex = nIndex;
	}

	int32_t CNetSocket::getSocketIndex() const
	{
		return this->m_nSocketIndex;
	}

	bool CNetSocket::open()
	{
		if (this->m_pNetEventLoop->getSocketCount() >= this->m_pNetEventLoop->getMaxSocketCount())
		{
			g_pLog->printWarning("out of max socket count %d", this->m_pNetEventLoop->getMaxSocketCount());
			return false;
		}

		this->m_nSocketID = (int32_t)::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_Invalid_SocketID == this->m_nSocketID)
		{
			g_pLog->printWarning("create socket error %d", getLastError());
			return false;
		}

		return true;
	}

	void CNetSocket::close(bool bCloseSend /* = true */)
	{
		if (this->m_bWaitClose)
			return;

		g_pLog->printInfo("CNetSocket::close socketid %d", this->m_nSocketID);

		if (bCloseSend)
			::shutdown(this->m_nSocketID, SD_SEND);

		this->m_pNetEventLoop->addCloseSocket(this);
		this->m_bWaitClose = true;
	}

	bool CNetSocket::nonblock()
	{
		DebugAstEx(this->m_nSocketID != _Invalid_SocketID, false);

#ifdef _WIN32
		int32_t nRet = 0;
		u_long nFlag = 1;
		nRet = ::ioctlsocket(this->m_nSocketID, FIONBIO, &nFlag);
		if (0 != nRet)
		{
			g_pLog->printWarning("set socket nonblock error %d", getLastError());
			closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
			return false;
		}
#else
		int32_t nFlag = ::fcntl(this->m_nSocketID, F_GETFL, 0);
		if (-1 == nFlag)
		{
			g_pLog->printWarning("set socket nonblock error %d", getLastError());
			::closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
			return false;
		}

		nFlag = ::fcntl(this->m_nSocketID, F_SETFL, nFlag | O_NONBLOCK);

		if (-1 == nFlag)
		{
			g_pLog->printWarning("set socket nonblock error %d", getLastError());
			closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
			return false;
		}
#endif
		return true;
	}

	// 主要用于监听SOCKET 为了让监听SOCKET关闭后处于TIME_WAIT状态后，可以马上绑定在同一个端口，不然需要等待一段时间
	bool CNetSocket::reuseAddr()
	{
		DebugAstEx(this->m_nSocketID != _Invalid_SocketID, false);

		uint32_t nFlag = 1;
		if (0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_REUSEADDR, (char*)&nFlag, sizeof(nFlag)))
		{
			g_pLog->printWarning("set socket to reuse addr mode error %d", getLastError());
			closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
			return false;
		}

		return true;
	}

	bool CNetSocket::setBufferSize()
	{
		DebugAstEx(this->m_nSocketID != _Invalid_SocketID, false);

		if (this->m_nRecvBufferSize != 0 && 0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_RCVBUF, (char*)&this->m_nRecvBufferSize, sizeof(this->m_nRecvBufferSize)))
		{
			g_pLog->printWarning("set socket recvbuf error %d", getLastError());
			closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
			return false;
		}

		if (this->m_nSendBufferSize != 0 && 0 != ::setsockopt(this->m_nSocketID, SOL_SOCKET, SO_SNDBUF, (char*)&this->m_nSendBufferSize, sizeof(this->m_nSendBufferSize)))
		{
			g_pLog->printWarning("set socket sendbuf error %d", getLastError());
			closesocket(this->m_nSocketID);
			this->m_nSocketID = _Invalid_SocketID;
			return false;
		}

		return true;
	}

	void CNetSocket::setRemoteAddr()
	{
		struct sockaddr remoteAddr;
		socklen_t nPeerAddrLen = sizeof(remoteAddr);
		::getpeername(this->m_nSocketID, &remoteAddr, &nPeerAddrLen);
		// 不能用::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		this->m_sRemoteAddr.nPort = ntohs((reinterpret_cast<sockaddr_in*>(&remoteAddr))->sin_port);
		strncpy(this->m_sRemoteAddr.szHost, inet_ntoa((reinterpret_cast<sockaddr_in*>(&remoteAddr))->sin_addr), _countof(this->m_sRemoteAddr.szHost));
	}

	void CNetSocket::setLocalAddr()
	{
		struct sockaddr localAddr;
		socklen_t nLocalAddrLen = sizeof(localAddr);
		::getsockname(this->m_nSocketID, &localAddr, &nLocalAddrLen);
		// 不能用::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		this->m_sLocalAddr.nPort = ntohs((reinterpret_cast<sockaddr_in*>(&localAddr))->sin_port);
		strncpy(this->m_sLocalAddr.szHost, inet_ntoa((reinterpret_cast<sockaddr_in*>(&localAddr))->sin_addr), _countof(this->m_sRemoteAddr.szHost));
	}

	void CNetSocket::setSocketID(int32_t nSocketID)
	{
		this->m_nSocketID = nSocketID;
	}

	int32_t CNetSocket::GetSocketID() const
	{
		return this->m_nSocketID;
	}

	void CNetSocket::disableRecv()
	{
		if ((this->m_nEvent&eNET_Recv) == 0)
			return;

		this->m_nEvent &= ~eNET_Recv;

#ifndef _WIN32
		this->m_pNetEventLoop->updateEpollState(this, EPOLL_CTL_MOD);
#endif
	}

	uint32_t CNetSocket::getEvent() const
	{
		return this->m_nEvent;
	}

	uint32_t CNetSocket::getSendBufferSize() const
	{
		return this->m_nSendBufferSize;
	}

	uint32_t CNetSocket::getRecvBufferSize() const
	{
		return this->m_nRecvBufferSize;
	}

}
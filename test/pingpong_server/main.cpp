#include "network.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#endif

#include <iostream>
using namespace std;

#pragma pack(push,1)
// 消息头
struct message_header
{
	uint16_t	nMessageSize;	// 包括消息头的
	uint16_t	nMessageID;
};
#pragma pack(pop)

class CNetConnecterHandler :
	public net::INetConnecterHandler
{
public:
	virtual uint32_t onRecv(const char* pData, uint32_t nDataSize)
	{
		uint32_t nRecvSize = 0;
		do
		{
			uint16_t nMessageSize = 0;

			// 都不够消息头
			if (nDataSize < sizeof(message_header))
				break;

			const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
			if (pHeader->nMessageSize < sizeof(message_header))
			{
				this->m_pNetConnecter->shutdown(true, "");
				break;
			}

			// 不是完整的消息
			if (nDataSize < pHeader->nMessageSize)
				break;

			nMessageSize = pHeader->nMessageSize;

			this->onDispatch(pHeader);

			nRecvSize += nMessageSize;
			nDataSize -= nMessageSize;
			pData += nMessageSize;

		} while (true);

		return nRecvSize;
	}

	virtual void onSendComplete(uint32_t nSize)
	{
	}

	void onDispatch(const message_header* pHeader)
	{
		std::cout << "onDispatch" << std::endl;
		this->m_pNetConnecter->send(pHeader, pHeader->nMessageSize, false);
	}

	virtual void onConnect()
	{
		std::cout << "onConnect" << std::endl;
	}

	virtual void onDisconnect()
	{
		std::cout << "onDisconnect" << std::endl;
	}
};

class CNetAccepterHandler :
	public net::INetAccepterHandler
{
public:
	virtual net::INetConnecterHandler* onAccept(net::INetConnecter* pNetConnecter)
	{
		return new CNetConnecterHandler();
	}
};

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#define _MAX_SOCKET_COUNT 200000

int main()
{
#ifndef _WIN32
	// 避免在向一个已经关闭的socket写数据导致进程终止
	struct sigaction sig_action;
	memset(&sig_action, 0, sizeof(sig_action));
	sig_action.sa_handler = SIG_IGN;
	sig_action.sa_flags = 0;
	sigaction(SIGPIPE, &sig_action, 0);

	// 避免终端断开发送SIGHUP信号导致进程终止
	memset(&sig_action, 0, sizeof(sig_action));
	sig_action.sa_handler = SIG_IGN;
	sig_action.sa_flags = 0;
	sigaction(SIGHUP, &sig_action, 0);

	struct rlimit rl;
	rl.rlim_cur = rl.rlim_max = _MAX_SOCKET_COUNT;
	if (::setrlimit(RLIMIT_NOFILE, &rl) == -1)
	{
		std::cout << "setrlimit error" << std::endl;
		return 0;
	}
#endif

	net::startupNetwork(nullptr);

	net::INetEventLoop* pNetEventLoop = net::createNetEventLoop();
	pNetEventLoop->init(_MAX_SOCKET_COUNT);

	CNetAccepterHandler* pNetAccepterHandler = new CNetAccepterHandler();
	SNetAddr netAddr;
	netAddr.nPort = 8000;
	strncpy(netAddr.szHost, "0.0.0.0", sizeof(netAddr.szHost));
	pNetEventLoop->listen(netAddr, 1024, 1024, pNetAccepterHandler);

	while (true)
	{
		pNetEventLoop->update(10);
	}

	return 0;
}

#ifdef _WIN32
#pragma warning(pop)
#endif
#include "network.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#endif

#include <iostream>
#include <thread>
using namespace std;

#pragma pack(push,1)
// 消息头
struct message_header
{
	uint16_t	nMessageSize;	// 包括消息头的
	uint16_t	nMessageID;
};

struct STestMsg : public message_header
{
	uint64_t	nTime;
};
#pragma pack(pop)

uint32_t g_nPackageSize = 0;
char g_szBuf[65536] = { 0 };

int64_t getProcessPassTime()
{
#ifdef _WIN32
	struct SFreq
	{
		LARGE_INTEGER	freq;
		int64_t			nFirstCheckTime;
		SFreq()
		{
			if (!QueryPerformanceFrequency(&freq))
			{
				fprintf(stderr, "cur machine cout support QueryPerformanceFrequency\n");
			}

			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);

			nFirstCheckTime = (counter.QuadPart * 1000000) / freq.QuadPart;
		}
	};

	static struct SFreq sFreq;

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	int64_t nTime = (counter.QuadPart * 1000000) / sFreq.freq.QuadPart;

	return nTime - sFreq.nFirstCheckTime;
#else
	// 在 x86-64 平台上，clock_gettime 不是系统调用，而是用vdso机制做的，就一个函数调用开销
	// 在编译链接时需加上 -lrt ;因为在librt中实现了clock_gettime函数
	// CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
	int64_t nTime = 0;
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		nTime = ((int64_t)ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
	// 上面不做强转的话如果编译成32位程序就会出现49天问题
	return nTime;
#endif
}

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
		STestMsg* netMsg = reinterpret_cast<STestMsg*>(g_szBuf);
		netMsg->nTime = getProcessPassTime();
		netMsg->nMessageID = 1000;
		netMsg->nMessageSize = (uint16_t)(sizeof(STestMsg) + g_nPackageSize);

		this->m_pNetConnecter->send(netMsg, netMsg->nMessageSize, false);

		std::cout << "onConnect" << std::endl;

// 		uint32_t i = 0;
// 		std::cin >> i;
// 		this->m_pNetConnecter->shutdown(false, "");
	}

	virtual void onDisconnect()
	{
		std::cout << "onDisconnect" << std::endl;
	}

	virtual void onConnectFail()
	{
		std::cout << "onConnectFail" << std::endl;
	}
};

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#define _MAX_SOCKET_COUNT 200000

int main(int argc, char* argv[])
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

	if (argc < 4)
	{
		std::cout << "arg error" << std::endl;
		return 0;
	}
	net::startupNetwork(nullptr);

	uint32_t nCount = atoi(argv[2]);
	g_nPackageSize = atoi(argv[3]);

	net::INetEventLoop* pNetEventLoop = net::createNetEventLoop();
	pNetEventLoop->init(_MAX_SOCKET_COUNT);

	SNetAddr netAddr;
	netAddr.nPort = 8000;
	strncpy(netAddr.szHost, argv[1], INET_ADDRSTRLEN);
	for (size_t i = 0; i < nCount; ++i)
	{
		pNetEventLoop->connect(netAddr, 1024, 1024, new CNetConnecterHandler());
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	while (true)
	{
		pNetEventLoop->update(10);
	}

	return 0;
}

#ifdef _WIN32
#pragma warning(pop)
#endif
#include "network.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include <iostream>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <map>

using namespace std;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#define ISspace(x) isspace((int)(x))
#ifndef _countof
#define _countof(elem) sizeof(elem)/sizeof(elem[0])
#endif

class CHttpConnection : public base::INetConnecterHandler
{
public:
	virtual uint32_t onRecv(const char* pData, uint32_t nDataSize);

	virtual void   onConnect();

	virtual void   onDisconnect();

	void send(const void* pData, uint32_t nSize, bool bCache);
};

namespace http
{
	std::map<std::string, std::string> mapCache;

	void not_found(CHttpConnection* pConnection)
	{
		static char szBuf[1024] = 
			"HTTP/1.0 404 NOT FOUND\r\n"\
			"Content-Type: text/html\r\n"\
			"\r\n"\
			"<HTML><TITLE>Not Found</TITLE>\r\n"\
			"<BODY><P>The server could not fulfill\r\n"\
			"your request because the resource specified\r\n"\
			"is unavailable or nonexistent.\r\n"\
			"</BODY></HTML>\r\n";

		pConnection->send(szBuf, (uint32_t)strlen(szBuf), false);
	}

	void bad_request(CHttpConnection* pConnection)
	{
		char szBuf[1024] =
			"HTTP/1.0 400 BAD REQUEST\r\n"\
			"Content-type: text/html\r\n"\
			"\r\n"\
			"<P>Your browser sent a bad request, "\
			"such as a POST without a Content-Length.\r\n";

		pConnection->send(szBuf, (uint32_t)strlen(szBuf), false);
	}

	void not_support(CHttpConnection* pConnection)
	{
		char szBuf[1024] =
			"HTTP/1.0 501 Method Not Support\r\n"\
			"Content-Type: text/html\r\n"\
			"\r\n"\
			"<HTML><HEAD><TITLE>Method Not Implemented\r\n"\
			"</TITLE></HEAD>\r\n"\
			"<BODY><P>HTTP request method not supported.\r\n"\
			"</BODY></HTML>\r\n";

		pConnection->send(szBuf, (uint32_t)strlen(szBuf), false);
	}

	void request(CHttpConnection* pConnection, const char* szData, uint32_t nSize)
	{
		char szMethod[256];
		uint32_t i = 0;
		for (; i < nSize && i < _countof(szMethod); ++i)
		{
			if (ISspace(szData[i]))
			{
				szMethod[i] = 0;
				break;
			}

			szMethod[i] = szData[i];
		}

		if (strncmp(szMethod, "GET", 3) != 0)
		{
			not_support(pConnection);
			return;
		}

		for (; i < nSize; ++i)
		{
			if (!ISspace(szData[i]))
				break;
		}

		char szUrl[256];
		for (uint32_t j = 0; i < nSize && i < _countof(szUrl); ++i, ++j)
		{
			if (ISspace(szData[i]))
			{
				szUrl[j] = 0;
				break;
			}

			szUrl[j] = szData[i];
		}

		if (strlen(szUrl) == 0)
		{
			bad_request(pConnection);
			return;
		}

		char szFile[256];
		if (strlen(szUrl) == 1 && szUrl[0] == '/')
		{
			strncpy(szFile, "html/index.html", _countof(szFile));
		}
		else
		{
			strncpy(szFile, "html", _countof(szFile));
			strncpy(szFile + 4, szUrl, _countof(szFile) - 4);
		}

		static char szBuf[1024] =
			"HTTP/1.0 200 OK\r\n"\
			"Content-Type: text/html\r\n"\
			"\r\n";

		auto iter = mapCache.find(szFile);
		if (iter != mapCache.end())
		{
			pConnection->send(szBuf, (uint32_t)strlen(szBuf), true);
			pConnection->send(iter->second.c_str(), (uint32_t)iter->second.size(), false);
			return;
		}

		FILE* pFile = fopen(szFile, "r");
		if (pFile == nullptr)
		{
			not_found(pConnection);
			return;
		}

		pConnection->send(szBuf, (uint32_t)strlen(szBuf), true);

		uint32_t nLoop = 0;
		while (true)
		{
			size_t nCount = fread(szBuf, 1, _countof(szBuf), pFile);
			pConnection->send(szBuf, (uint32_t)nCount, false);
			++nLoop;
			if (nCount != _countof(szBuf))
			{
				if (nLoop == 1)
					mapCache[szFile] = szBuf;
				break;
			}
		}

		fclose(pFile);
	}
}

uint32_t CHttpConnection::onRecv(const char* pData, uint32_t nDataSize)
{
	if (nDataSize < 4)
		return 0;

	bool bFull = false;
	for (uint32_t i = 0; i < nDataSize - 3; ++i)
	{
		if (pData[i] == '\r' && pData[i + 1] == '\n'
			&& pData[i + 2] == '\r' && pData[i + 3] == '\n')
		{
			bFull = true;
			break;
		}
	}

	if (!bFull)
		return 0;

	http::request(this, pData, nDataSize);
	this->m_pNetConnecter->shutdown(false, "");
	return nDataSize;
}

void CHttpConnection::onConnect()
{

}

void CHttpConnection::onDisconnect()
{
}

void CHttpConnection::send(const void* pData, uint32_t nSize, bool bCache)
{
	this->m_pNetConnecter->send(pData, nSize, bCache);
}

class CHttpAccepterHandler : public base::INetAccepterHandler
{
public:
	virtual base::INetConnecterHandler* onAccept(base::INetConnecter* pNetConnecter)
	{
		return new CHttpConnection();
	}
};

class CDefaultLog : public base::INetworkLog
{
public:
	virtual void printInfo(const char* szFormat, ...)
	{
		
	}

	virtual void printWarning(const char* szFormat, ...)
	{
		uint8_t nDay = 0;
		char szBuf[4096] = { 0 };
		va_list arg;
		va_start(arg, szFormat);
		vsnprintf(szBuf, _countof(szBuf), szFormat, arg);
		printf("[Warning]%s\n", szBuf);
		va_end(arg);
	}
};

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
#endif

	CDefaultLog log;
	base::startupNetwork(&log);

	base::INetEventLoop* pNetEventLoop = base::createNetEventLoop();
	pNetEventLoop->init(50000);

	CHttpAccepterHandler* pHttpAccepterHandler = new CHttpAccepterHandler();
	SNetAddr netAddr;
	netAddr.nPort = 8000;
	strncpy(netAddr.szHost, "0.0.0.0", sizeof(netAddr.szHost));
	pNetEventLoop->listen(netAddr, 10 * 1024, 10 * 1024, pHttpAccepterHandler);
	while (true)
	{
		pNetEventLoop->update(10);
	}

	return 0;
}

#ifdef _WIN32
#pragma warning(pop)
#endif
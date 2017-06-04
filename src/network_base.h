#pragma once

#include <assert.h>
#include <stdint.h>

#ifdef _WIN32
#undef FD_SETSIZE
#define FD_SETSIZE 1024
#include <winsock2.h> // 这个头文件必须放在winsock.h前面，winsock.h有几个另外的头文件包含他
#include <windows.h>
#include <ws2tcpip.h>
#include <errno.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#endif

namespace net
{

#ifdef _WIN32

#	define MSG_NOSIGNAL 0
#	define _Invalid_SocketID (int32_t)INVALID_SOCKET


#	define NW_EWOULDBLOCK		WSAEWOULDBLOCK
#	define NW_ECONNABORTED		WSAECONNABORTED
#	define NW_EPROTO			EPROTO
#	define NW_EINPROGRESS		WSAEINPROGRESS
#	define NW_EAGAIN			EAGAIN
#	define NW_EINTR				EINTR
#	define NW_EALREADY			WSAEALREADY
#	define NW_ENOTSOCK			WSAENOTSOCK
#	define NW_EDESTADDRREQ		WSAEDESTADDRREQ
#	define NW_EMSGSIZE			WSAEMSGSIZE
#	define NW_EPROTOTYPE		WSAEPROTOTYPE
#	define NW_ENOPROTOOPT		WSAENOPROTOOPT
#	define NW_EPROTONOSUPPO		WSAEPROTONOSUPPORT
#	define NW_ESOCKTNOSUPPO		WSAESOCKTNOSUPPORT
#	define NW_EOPNOTSUPP		WSAEOPNOTSUPP
#	define NW_EPFNOSUPPORT		WSAEPFNOSUPPORT
#	define NW_EAFNOSUPPORT		WSAEAFNOSUPPORT
#	define NW_EADDRINUSE		WSAEADDRINUSE
#	define NW_EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define NW_ENETDOWN			WSAENETDOWN
#	define NW_ENETUNREACH		WSAENETUNREACH
#	define NW_ENETRESET			WSAENETRESET
#	define NW_ECONNABORTED		WSAECONNABORTED
#	define NW_ECONNRESET		WSAECONNRESET
#	define NW_ENOBUFS			WSAENOBUFS
#	define NW_EISCONN			WSAEISCONN
#	define NW_ENOTCONN			WSAENOTCONN
#	define NW_ESHUTDOWN			WSAESHUTDOWN
#	define NW_ETOOMANYREFS		WSAETOOMANYREFS
#	define NW_ETIMEDOUT			WSAETIMEDOUT
#	define NW_ECONNREFUSED		WSAECONNREFUSED
#	define NW_ELOOP				WSAELOOP
#	define NW_EHOSTDOWN			WSAEHOSTDOWN
#	define NW_EHOSTUNREACH		WSAEHOSTUNREACH
#	define NW_EPROCLIM			WSAEPROCLIM
#	define NW_EUSERS			WSAEUSERS
#	define NW_EDQUOT			WSAEDQUOT
#	define NW_ESTALE			WSAESTALE
#	define NW_EREMOTE			WSAEREMOTE

#else

#	define _Invalid_SocketID -1
#	define closesocket close
#	define SOCKET_ERROR  -1
#	define SD_SEND SHUT_WR


#	define NW_EWOULDBLOCK		EWOULDBLOCK
#	define NW_ECONNABORTED		ECONNABORTED
#	define NW_EPROTO			EPROTO
#	define NW_EINPROGRESS		EINPROGRESS
#	define NW_EAGAIN			EAGAIN
#	define NW_EINTR				EINTR
#	define NW_EALREADY			EALREADY
#	define NW_ENOTSOCK			ENOTSOCK
#	define NW_EDESTADDRREQ		EDESTADDRREQ
#	define NW_EMSGSIZE			EMSGSIZE
#	define NW_EPROTOTYPE		EPROTOTYPE
#	define NW_ENOPROTOOPT		ENOPROTOOPT
#	define NW_EPROTONOSUPPO		EPROTONOSUPPORT
#	define NW_ESOCKTNOSUPPO		ESOCKTNOSUPPORT
#	define NW_EOPNOTSUPP		EOPNOTSUPP
#	define NW_EPFNOSUPPORT		EPFNOSUPPORT
#	define NW_EAFNOSUPPORT		EAFNOSUPPORT
#	define NW_EADDRINUSE		EADDRINUSE
#	define NW_EADDRNOTAVAIL		EADDRNOTAVAIL
#	define NW_ENETDOWN			ENETDOWN
#	define NW_ENETUNREACH		ENETUNREACH
#	define NW_ENETRESET			ENETRESET
#	define NW_ECONNABORTED		ECONNABORTED
#	define NW_ECONNRESET		ECONNRESET
#	define NW_ENOBUFS			ENOBUFS
#	define NW_EISCONN			EISCONN
#	define NW_ENOTCONN			ENOTCONN
#	define NW_ESHUTDOWN			ESHUTDOWN
#	define NW_ETOOMANYREFS		ETOOMANYREFS
#	define NW_ETIMEDOUT			ETIMEDOUT
#	define NW_ECONNREFUSED		ECONNREFUSED
#	define NW_ELOOP				ELOOP
#	define NW_EHOSTDOWN			EHOSTDOWN
#	define NW_EHOSTUNREACH		EHOSTUNREACH
#	define NW_EPROCLIM			EPROCLIM
#	define NW_EUSERS			EUSERS
#	define NW_EDQUOT			EDQUOT
#	define NW_ESTALE			ESTALE
#	define NW_EREMOTE			EREMOTE
#	define NW_EMFILE			EMFILE

#endif

#define _Invalid_SendConnecterIndex -1
#define _Invalid_SocketIndex -1


	enum ENetEventType
	{
#ifdef _WIN32
		eNET_Recv	= 1,
		eNET_Send	= 2,
#else
		/*
			EPOLLIN		有新连接进来触发 对端普通数据进来触发 对端正常关闭连接触发（此时还可能触发EPOLLRDHUP）
			EPOLLOUT	水平触发模式下，只要发送缓冲区没满就一直触发，边沿触发模式下发送缓冲区从高水位进入低水位时触发（水位可以设置默认是1）
			EPOLLERR	socket能检测到对方出错吗？目前为止，好像我还不知道如何检测，但是，在给已经关闭的socket写时，会发生EPOLLERR，也就是说，
						只有在采取行动（比如读一个已经关闭的socket，或者写一个已经关闭的socket）时候，才知道对方是否关闭了。这个时候如果对方关闭了，
						则会出现EPOLLERR，EPOLLERR是服务器这边出错
						EPOLLERR|EPOLLHUP 这两个标记epoll_wait会默认检测，不需要设置
						EPOLLRDHUP 这个在有些系统中表示对端已经关闭，在我们库里面完全可以由EPOLLIN事件来判断，所以没有用
		*/
		eNET_Recv	= EPOLLIN|POLLPRI,
		eNET_Send	= EPOLLOUT,
#endif
		eNET_Unknown = 0xff
	};

#define _SEND_BUF_SIZE	2048
#define _RECV_BUF_SIZE	2048

#define	DebugAst(Exp)			do{ if(!(Exp)) { assert(0); return; } } while(0)
#define	DebugAstEx(Exp, Ret)	do{ if(!(Exp)) { assert(0); return Ret; } } while(0)


#ifndef _countof
#define _countof(elem) sizeof(elem)/sizeof(elem[0])
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif
	// 如果目标字符串足以放下格式化后字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
	inline size_t snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...)
	{
		if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
			return -1;

		va_list arg;
		va_start(arg, szFormat);
		size_t ret = vsnprintf(szBuf, nBufSize, szFormat, arg);
		va_end(arg);

		return ret;
	}
	// 如果目标字符串足以放下格式化后字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
	inline size_t vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg)
	{
		if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
			return -1;

#ifdef _WIN32
		// 这里用旧的函数是为了跟linux相同语义
		int32_t nRet = ::_vsnprintf(szBuf, nBufSize, szFormat, arg);
		if (nRet == nBufSize)
		{
			szBuf[nBufSize - 1] = 0;
			--nRet;
		}
		return nRet;
#else
		return ::vsnprintf(szBuf, nBufSize, szFormat, arg);
#endif
	}

	inline uint32_t getLastError() 
	{
#ifdef _WIN32
		return (uint32_t)GetLastError();
#else
		return (uint32_t)errno;
#endif
	}
#ifdef _WIN32
#pragma warning(pop)
#endif
}
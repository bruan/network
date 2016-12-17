#include "stdafx.h"
#include "net_wakeup.h"

#ifndef _WIN32
#include <sys/eventfd.h>
#endif

namespace net
{
	CNetWakeup::CNetWakeup()
		: m_nWakeup(-1)
		, m_pNetEventLoop(nullptr)
		, m_flag(false)
	{

	}

	CNetWakeup::~CNetWakeup()
	{
#ifndef _WIN32
		::close(this->m_nWakeup);
#endif
	}

	bool CNetWakeup::init(CNetEventLoop* pNetEventLoop)
	{
		DebugAstEx(pNetEventLoop != nullptr && this->m_pNetEventLoop == nullptr, false);

#ifndef _WIN32
		this->m_nWakeup = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (this->m_nWakeup < 0)
			return false;
		struct epoll_event event;
		memset(&event, 0, sizeof(event));
		event.data.ptr = this;
		event.events = EPOLLIN;
		if (epoll_ctl(pNetEventLoop->getEpoll(), EPOLL_CTL_ADD, this->m_nWakeup, &event) < 0)
			return false;
#endif

		this->m_pNetEventLoop = pNetEventLoop;
		
		return true;
	}

	void CNetWakeup::onEvent(uint32_t nEvent)
	{
#ifndef _WIN32
		if (nEvent&eNET_Recv)
		{
			uint64_t nData = 1;
			int32_t nRet = ::read(this->m_nWakeup, &nData, sizeof(nData));
			if (nRet != sizeof(nData))
			{
				g_pLog->printWarning("read event fd err ret: %d", nRet);
			}
		}
#endif
	}

	void CNetWakeup::wakeup()
	{
#ifndef _WIN32
		if (!this->m_flag.load(std::memory_order_acquire))
			return;

		uint64_t nData = 1;
		int32_t nRet = ::write(this->m_nWakeup, &nData, sizeof(nData));
		if (nRet != sizeof(nData))
		{
			g_pLog->printWarning("write event fd err ret: %d", nRet);
		}

		this->wait(false);
#endif
	}

	void CNetWakeup::wait(bool flag)
	{
		this->m_flag.store(flag, std::memory_order_release);
	}

}
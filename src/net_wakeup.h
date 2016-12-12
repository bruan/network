#pragma once
#include "net_socket.h"
#include <atomic>

namespace base
{
	class CNetWakeup :
		public INetBase
	{
	public:
		CNetWakeup();
		virtual ~CNetWakeup();

		bool			init(CNetEventLoop* pNetEventLoop);

		virtual void	onEvent(uint32_t nEvent);
		
		void			wait(bool flag);
		void			wakeup();

	private:
		CNetEventLoop*		m_pNetEventLoop;
		int32_t				m_nWakeup;
		std::atomic<bool>	m_flag;
	};
}
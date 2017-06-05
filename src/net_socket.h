#pragma once

#include "net_event_loop.h"
#include "network.h"

namespace net
{
	/**
	@brief: 网络基础类
	*/
	class INetBase
	{
	public:
		virtual ~INetBase() { }

		/**
		@brief: 事件回调
		*/
		virtual void	onEvent(uint32_t nEvent) = 0;
	};

	class CNetSocket :
		public INetBase
	{
		friend class CNetEventLoop;

	public:
		enum ENetSocketType
		{
			eNST_Acceptor,
			eNST_Connector,
		};

	public:
		CNetSocket();
		~CNetSocket();

		virtual bool		init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);
		virtual void		release();
		virtual void		onEvent(uint32_t nEvent) = 0;
		virtual uint32_t	getSocketType() const = 0;
		virtual bool		isDisableWrite() const = 0;

		bool				open();
		bool				nonblock();
		bool				reuseAddr();
		bool				reusePort();
		bool				setNoDelay(bool bEnable);
		bool				setBufferSize();
		void				setRemoteAddr();
		void				setLocalAddr();
		void				setSocketID(int32_t nSocketID);
		int32_t				GetSocketID() const;
		void				setSocketIndex(int32_t nIndex);
		int32_t				getSocketIndex() const;
		uint32_t			getSendBufferSize() const;
		uint32_t			getRecvBufferSize() const;

	protected:
		void				close(bool bRelease, bool bCloseSend);

	protected:
		CNetEventLoop*	m_pNetEventLoop;
		SNetAddr		m_sLocalAddr;
		SNetAddr		m_sRemoteAddr;

	private:
		int32_t			m_nSocketID;
		int32_t			m_nSocketIndex;
		uint32_t		m_nSendBufferSize;
		uint32_t		m_nRecvBufferSize;
		bool			m_bWaitClose;
	};
}
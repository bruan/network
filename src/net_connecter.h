#pragma once

#include "net_socket.h"
#include "network.h"
#include "net_buffer.h"

namespace net
{
	class CNetConnecter :
		public INetConnecter,
		public CNetSocket
	{
	public:
		enum ENetConnecterFlag
		{
			eNCF_CloseRecv		= 1<<0,
			eNCF_CloseSend		= 1<<1,
			eNCF_DisableWrite	= 1<<2,
			eNCF_ConnectFail	= 1<<3,
		};

	public:
		CNetConnecter();
		virtual ~CNetConnecter();

		virtual bool				init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);
		virtual void				release();
		virtual void				onEvent(uint32_t nEvent);
		virtual uint32_t			getSocketType() const { return eNST_Connector; }
		virtual bool				isDisableWrite() const;

		virtual bool				send(const void* pData, uint32_t nDataSize, bool bCache);
		virtual void				setHandler(INetConnecterHandler* pHandler);
		virtual void				shutdown(bool bForce, const char* szFormat, ...);
		virtual const SNetAddr&		getLocalAddr() const;
		virtual const SNetAddr&		getRemoteAddr() const;
		virtual ENetConnecterType	getConnecterType() const;
		virtual ENetConnecterState	getConnecterState() const;
		virtual	uint32_t			getSendDataSize() const;
		virtual	uint32_t			getRecvDataSize() const;
		virtual bool				setNoDelay(bool bEnable);

		void						printInfo(const char* szFormat, ...);
		int32_t						getSendConnecterIndex() const;
		void						setSendConnecterIndex(int32_t nIndex);
		bool						connect(const SNetAddr& sNetAddr);
		void						setConnecterType(ENetConnecterType eConnecterType);
		void						setConnecterState(ENetConnecterState eConnecterState);
		void						flushSend();

	private:
		void						onConnect();
		void						onRecv();
		void						onSend();

	private:
		ENetConnecterType		m_eConnecterType;
		ENetConnecterState		m_eConnecterState;
		uint32_t				m_nFlag;
		CNetRecvBuffer*			m_pRecvBuffer;
		CNetSendBuffer*			m_pSendBuffer;
		INetConnecterHandler*	m_pHandler;
		uint32_t				m_nSendConnecterIndex;
	};
}
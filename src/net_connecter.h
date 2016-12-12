#pragma once

#include "net_socket.h"
#include "network.h"
#include "net_buffer.h"

namespace base
{
	class CNetConnecter :
		public INetConnecter,
		public CNetSocket
	{
	public:
		enum ECloseType
		{
			eCT_None = 0,
			eCT_Recv = 1,
			eCT_Send = 2,
		};

	public:
		CNetConnecter();
		virtual ~CNetConnecter();

		virtual void				onEvent(uint32_t nEvent);
		virtual void				forceClose();
		virtual bool				init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);

		virtual void				send(const void* pData, uint32_t nDataSize, bool bCache);
		virtual void				setHandler(INetConnecterHandler* pHandler);
		virtual void				shutdown(bool bForce, const char* szMsg);
		virtual const SNetAddr&		getLocalAddr() const;
		virtual const SNetAddr&		getRemoteAddr() const;
		virtual ENetConnecterType	getConnecterType() const;
		virtual ENetConnecterState	getConnecterState() const;
		virtual	uint32_t			getSendDataSize() const;
		virtual	uint32_t			getRecvDataSize() const;

		void						printInfo(const char* szMsg);
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
		uint32_t				m_nCloseType;
		CNetRecvBuffer*			m_pRecvBuffer;
		CNetSendBuffer*			m_pSendBuffer;
		INetConnecterHandler*	m_pHandler;
		uint32_t				m_nSendConnecterIndex;
	};
}
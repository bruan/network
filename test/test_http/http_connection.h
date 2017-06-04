#pragma once
#include "network.h"
#include "http_request.h"

namespace http
{
	class CHttpServer;
	class CHttpConnection : 
		public net::INetConnecterHandler
	{
	public:
		CHttpConnection(CHttpServer* pHttpServer);
		virtual ~CHttpConnection();

		virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize);
		virtual void		onSendComplete(uint32_t nSize) { }
		virtual void		onConnect();
		virtual void		onDisconnect();
		virtual void		onConnectFail() { }

	private:
		bool				parseRequestLine(const char* szBegin, const char* szEnd);
		bool				parseRequest(const char* szData, uint32_t nDataSize);

	private:
		enum EParseState
		{
			ePS_RequestLine,
			ePS_Headers,
			ePS_Body,
			ePS_Finish,
		};

		EParseState		m_nState;
		CHttpRequest	m_request;
		uint32_t		m_nParseDataCount;
		CHttpServer*	m_pHttpServer;
	};
}
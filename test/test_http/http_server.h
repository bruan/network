#pragma once
#include "http_base.h"
#include "network.h"
#include <functional>
#include <map>

namespace http
{

	class CHttpRequest;
	class CHttpResponse;
	class CHttpConnection;

	class CHttpServer
	{
		friend class CHttpConnection;

	public:
		CHttpServer();
		~CHttpServer();

		bool	start(const SNetAddr& sListenAddr, uint32_t nConnectionCount);
		void	registerCallback(const std::string& szName, const HttpCallback& callback);
		void	registerDir(const std::string& szName, const std::string& szDir);
	
	private:
		bool	dispatch(CHttpConnection* pHttpConnection, const CHttpRequest& sRequest);

	private:
		 net::INetEventLoop*					m_pNetEventLoop;
		 std::map<std::string, HttpCallback>	m_mapCallback;
		 std::string							m_szBuf;
	};
}

#include "http_server.h"
#include "http_connection.h"
#include "http_request.h"
#include "http_response.h"
#include "file_system.h"

namespace http
{
	class CNetAccepterHandler :
		public net::INetAccepterHandler
	{
	public:
		CNetAccepterHandler(CHttpServer* pHttpServer) : m_pHttpServer(pHttpServer) { }

		virtual net::INetConnecterHandler* onAccept(net::INetConnecter* pNetConnecter)
		{
			return new CHttpConnection(this->m_pHttpServer);
		}

	private:
		CHttpServer* m_pHttpServer;
	};

	CHttpServer::CHttpServer()
		: m_pNetEventLoop(nullptr)
	{
	}

	CHttpServer::~CHttpServer()
	{
	}

	bool CHttpServer::start(const SNetAddr& sListenAddr, uint32_t nConnectionCount)
	{
		net::INetEventLoop* pNetEventLoop = net::createNetEventLoop();
		pNetEventLoop->init(nConnectionCount);

		CNetAccepterHandler* pNetAccepterHandler = new CNetAccepterHandler(this);
		if (!pNetEventLoop->listen(sListenAddr, 1024, 1024, pNetAccepterHandler))
			return false;

		while (true)
		{
			pNetEventLoop->update(10);
		}

		return true;
	}

	bool CHttpServer::dispatch(CHttpConnection* pHttpConnection, const CHttpRequest& sRequest)
	{
		const std::string& szRequestURI = sRequest.getRequestURI();
		CHttpResponse sResponse;
		auto iter = this->m_mapCallback.find(szRequestURI);
		if (iter != this->m_mapCallback.end())
		{
			const std::string& connection = sRequest.getHeader("Connection");
			bool bClose = connection == "close" || (sRequest.getVersion() == eV_Http10 && connection != "Keep-Alive");
			sResponse.setKeepAlive(true);
			sResponse.setStatusCode(eSC_200_Ok);
			iter->second(sRequest, sResponse);
		}
		else
		{
			std::string::size_type pos = szRequestURI.find_last_of('/');
			if (pos != std::string::npos)
			{
				std::string szPath = szRequestURI.substr(0, pos);
				iter = this->m_mapCallback.find(szPath);
				if (iter != this->m_mapCallback.end())
				{
					const std::string& connection = sRequest.getHeader("Connection");
					bool bClose = connection == "close" || (sRequest.getVersion() == eV_Http10 && connection != "Keep-Alive");
					sResponse.setKeepAlive(true);
					sResponse.setStatusCode(eSC_200_Ok);
					iter->second(sRequest, sResponse);
				}
				else
					pos = std::string::npos;
			}

			if (pos == std::string::npos)
			{
				sResponse.setStatusCode(eSC_404_NotFound);
				sResponse.setStatusMessage("Not Found");
				sResponse.setKeepAlive(false);

				sResponse.write("404 page not found\n");
			}
		}

		sResponse.serialize(this->m_szBuf);
		pHttpConnection->send(this->m_szBuf.c_str(), (uint32_t)this->m_szBuf.size(), false);

		return sResponse.isKeepAlive();
	}

	void CHttpServer::registerCallback(const std::string& szName, const HttpCallback& callback)
	{
		this->m_mapCallback[szName] = callback;
	}

	void CHttpServer::registerDir(const std::string& szName, const std::string& szDir)
	{
		std::string szLogicName;
		std::string::size_type pos = szName.find_last_of('/');
		if (pos == szName.size() - 1)
			szLogicName = szName.substr(0, pos);
		else
			szLogicName = szName;

		auto callback = [szLogicName, szDir](const CHttpRequest& sRequest, CHttpResponse& sResponse)->void
		{
			CFileSystem::Inst()->handleFile(szLogicName, szDir, sRequest, sResponse);
		};
		this->registerCallback(szLogicName, callback);
	}
}
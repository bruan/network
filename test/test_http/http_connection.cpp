#include "http_connection.h"
#include "http_server.h"

#include <algorithm>

namespace http
{
	const char zCRLF[] = "\r\n";

	CHttpConnection::CHttpConnection(CHttpServer* pHttpServer)
		: m_pHttpServer(pHttpServer)
		, m_nState(ePS_RequestLine)
		, m_nParseDataCount(0)
	{

	}

	CHttpConnection::~CHttpConnection()
	{

	}

	uint32_t CHttpConnection::onRecv(const char* pData, uint32_t nDataSize)
	{
		if (!this->parseRequest(pData, nDataSize))
		{
			static char szBuf[] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n400 Bad Request\n";

			this->m_pNetConnecter->send(szBuf, sizeof(szBuf), false);
			this->m_pNetConnecter->shutdown(false, "bad request");
			return nDataSize;
		}

		if (this->m_nState == ePS_Finish)
		{
			if (!this->m_pHttpServer->dispatch(this, this->m_request))
				this->m_pNetConnecter->shutdown(false, "");

			this->m_request.reset();
			this->m_nState = ePS_RequestLine;

			return this->m_nParseDataCount;
		}

		return 0;
	}

	void CHttpConnection::onConnect()
	{

	}

	void CHttpConnection::onDisconnect()
	{

	}

	bool CHttpConnection::parseRequestLine(const char* szBegin, const char* szEnd)
	{
		const char* szSpace = std::find(szBegin, szEnd, ' ');
		if (szSpace == szEnd)
			return false;

		if (!this->m_request.setMethod(szBegin, szSpace))
			return false;

		szBegin = szSpace + 1;
		szSpace = std::find(szBegin, szEnd, ' ');
		if (szSpace == szEnd)
			return false;

		const char* szQuery = std::find(szBegin, szSpace, '?');
		if (szQuery != szSpace)
		{
			this->m_request.setRequestURI(szBegin, szQuery);
			this->m_request.setQuery(szQuery + 1, szSpace);
		}
		else
		{
			this->m_request.setRequestURI(szBegin, szSpace);
		}

		szBegin = szSpace + 1;
		bool ok = szEnd - szBegin == 8 && std::equal(szBegin, szEnd - 1, "HTTP/1.");
		if (!ok)
			return false;

		if (*(szEnd - 1) == '1')
			this->m_request.setVersion(eV_Http11);
		else if (*(szEnd - 1) == '0')
			this->m_request.setVersion(eV_Http10);
		else
			return false;

		return true;
	}

	bool CHttpConnection::parseRequest(const char* szData, uint32_t nDataSize)
	{
		while (true)
		{
			if (this->m_nState == ePS_RequestLine)
			{
				const char* szCRLF = std::search(szData, szData + nDataSize, zCRLF, zCRLF + 2);
				if (szCRLF == szData + nDataSize)
					break;

				bool ok = parseRequestLine(szData, szCRLF);
				if (!ok)
					return false;

				this->m_nParseDataCount = (uint32_t)(szCRLF - szData + 2);
				this->m_nState = ePS_Headers;
			}
			else if (this->m_nState == ePS_Headers)
			{
				const char* szBegin = szData + this->m_nParseDataCount;
				const char* szEnd = szData + nDataSize;
				const char* szCRLF = std::search(szBegin, szEnd, zCRLF, zCRLF + 2);
				if (szCRLF == szEnd)
					break;

				const char* pColom = std::find(szBegin, szCRLF, ':');
				if (pColom != szCRLF)
				{
					this->m_request.addHeader(szBegin, pColom, szCRLF);
				}
				else
				{
					this->m_nState = ePS_Body;
				}
				this->m_nParseDataCount += (uint32_t)(szCRLF - szBegin + 2);
			}
			else if (this->m_nState == ePS_Body)
			{
				if (this->m_request.getMethod() == eMT_Get)
				{
					this->m_nState = ePS_Finish;
					break;
				}

				std::string szLength = this->m_request.getHeader("Content-Length");
				if (szLength.empty())
				{
					this->m_nState = ePS_Finish;
					break;
				}

				int32_t nLength = atoi(szLength.c_str());
				if (nLength < 0)
					return false;

				if (nLength + this->m_nParseDataCount > nDataSize)
					return false;

				const char* szBegin = szData + this->m_nParseDataCount;
				const char* szEnd = szBegin + nLength;
				this->m_request.setBody(szBegin, szEnd);
				this->m_nParseDataCount += nLength;

				this->m_nState = ePS_Finish;
				break;
			}
		}
		return true;
	}
}
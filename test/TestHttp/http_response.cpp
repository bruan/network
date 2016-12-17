#include "http_response.h"

namespace http
{
	CHttpResponse::CHttpResponse()
		: m_nStatusCode(eSC_Unknown)
	{

	}

	CHttpResponse::~CHttpResponse()
	{

	}

	void CHttpResponse::serialize(std::string& szData) const
	{
		char szBuf[64];
		snprintf(szBuf, sizeof(szBuf), "HTTP/1.1 %d ", this->m_nStatusCode);
		szData = szBuf;
		szData += this->m_nStatusMessage;
		szData += "\r\n";

		if (this->m_bKeepAlive)
		{
			snprintf(szBuf, sizeof(szBuf), "Content-Length: %d\r\n", m_szBody.size());
			szData += szBuf;
			szData += "Connection: Keep-Alive\r\n";
		}
		else
		{
			szData += "Connection: close\r\n";
		}

		for (auto iter = this->m_mapHeader.begin(); iter != m_mapHeader.end(); ++iter)
		{
			szData += iter->first;
			szData += ": ";
			szData += iter->second;
			szData += "\r\n";
		}

		szData += "\r\n";
		szData += this->m_szBody;
	}

	void CHttpResponse::setContentType(const std::string& szContentType)
	{
		this->addHeader("Content-Type", szContentType);
	}

	void CHttpResponse::setStatusCode(EStatusCode eStatusCode)
	{
		this->m_nStatusCode = eStatusCode;
	}

	void CHttpResponse::setStatusMessage(const std::string& szMessage)
	{
		this->m_nStatusMessage = szMessage;
	}

	void CHttpResponse::setKeepAlive(bool bEnable)
	{
		this->m_bKeepAlive = bEnable;
	}

	bool CHttpResponse::isKeepAlive() const
	{
		return this->m_bKeepAlive;
	}

	void CHttpResponse::addHeader(const std::string& szKey, const std::string& szValue)
	{
		this->m_mapHeader[szKey] = szValue;
	}

	void CHttpResponse::write(const char* szData)
	{
		this->m_szBody.append(szData);
	}

	void CHttpResponse::write(const char* szData, uint32_t nSize)
	{
		this->m_szBody.append(szData, nSize);
	}
}
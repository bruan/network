#include "http_request.h"

namespace http
{
	CHttpRequest::CHttpRequest() 
		: m_eMethod(eMT_Unknown)
		, m_eVersion(eV_Unknown)
	{

	}

	CHttpRequest::~CHttpRequest()
	{

	}

	bool CHttpRequest::setMethod(const char* start, const char* end)
	{
		std::string szMethod(start, end);
		if (szMethod == "GET")
		{
			this->m_eMethod = eMT_Get;
		}
		else if (szMethod == "POST")
		{
			this->m_eMethod = eMT_Post;
		}
		else if (szMethod == "PUT")
		{
			this->m_eMethod = eMT_Put;
		}
		else if (szMethod == "DELETE")
		{
			this->m_eMethod = eMT_Delete;
		}
		else
		{
			this->m_eMethod = eMT_Unknown;
		}

		return this->m_eMethod != eMT_Unknown;
	}

	void CHttpRequest::setBody(const char* start, const char* end)
	{
		this->m_szBody = std::string(start, end);
	}

	void CHttpRequest::addHeader(const char* start, const char* colon, const char* end)
	{
		std::string field(start, colon);
		++colon;
		while (colon < end && isspace(*colon))
		{
			++colon;
		}
		std::string value(colon, end);
		while (!value.empty() && isspace(value[value.size() - 1]))
		{
			value.resize(value.size() - 1);
		}
		m_mapHeader[field] = value;
	}

	std::string CHttpRequest::getHeader(const std::string& field) const
	{
		std::string result;
		auto it = m_mapHeader.find(field);
		if (it != m_mapHeader.end())
		{
			result = it->second;
		}
		return result;
	}

	void CHttpRequest::reset()
	{
		this->m_eMethod = eMT_Unknown;
		this->m_eVersion = eV_Unknown;
		this->m_szRequestURI.clear();
		this->m_szQuery.clear();
		this->m_szBody.clear();
		this->m_mapHeader.clear();
	}

	void CHttpRequest::setVersion(EVersion eVersion)
	{
		this->m_eVersion = eVersion;
	}

	EVersion CHttpRequest::getVersion() const
	{
		return this->m_eVersion;
	}

	EMethodType CHttpRequest::getMethod() const
	{
		return this->m_eMethod;
	}

	void CHttpRequest::setRequestURI(const char* start, const char* end)
	{
		this->m_szRequestURI.assign(start, end);
	}

	const std::string& CHttpRequest::getRequestURI() const
	{
		return this->m_szRequestURI;
	}

	void CHttpRequest::setQuery(const char* start, const char* end)
	{
		this->m_szQuery.assign(start, end);
	}

	const std::string& CHttpRequest::getQuery() const
	{
		return m_szQuery;
	}

	const std::map<std::string, std::string>& CHttpRequest::getHeaders() const
	{
		return this->m_mapHeader;
	}

}
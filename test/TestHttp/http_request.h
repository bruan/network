#pragma once

#include "http_base.h"

#include <map>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

namespace http
{
	class CHttpRequest
	{
	public:
		CHttpRequest();
		~CHttpRequest();
		
		void				setVersion(EVersion eVersion);
		EVersion			getVersion() const;
		bool				setMethod(const char* start, const char* end);
		EMethodType			getMethod() const;
		void				setRequestURI(const char* start, const char* end);
		const std::string&	getRequestURI() const;
		void				setQuery(const char* start, const char* end);
		const std::string&	getQuery() const;
		void				setBody(const char* start, const char* end);
		const std::string&	getBody() const;
		void				addHeader(const char* start, const char* colon, const char* end);
		std::string			getHeader(const std::string& field) const;
		
		const std::map<std::string, std::string>&
							getHeaders() const;
		void				reset();

	private:
		EMethodType							m_eMethod;
		EVersion							m_eVersion;
		std::string							m_szRequestURI;
		std::string							m_szQuery;
		std::string							m_szBody;
		std::map<std::string, std::string>	m_mapHeader;
	};

}

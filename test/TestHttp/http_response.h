#pragma once

#include "http_base.h"

#include <map>

namespace http
{
	class CHttpResponse
	{
	public:
		CHttpResponse();
		~CHttpResponse();
		
		void setStatusCode(EStatusCode eStatusCode);
		void setStatusMessage(const std::string& szMessage);
		void setKeepAlive(bool bEnable);
		bool isKeepAlive() const;
		void setContentType(const std::string& szContentType);
		void addHeader(const std::string& szKey, const std::string& szValue);
		void write(const char* szData);
		void write(const char* szData, uint32_t nSize);
		void redirect();
		
		void serialize(std::string& szData) const;

	 private:
		 std::map<std::string, std::string> m_mapHeader;
		 EStatusCode						m_nStatusCode;
		 std::string						m_nStatusMessage;
		 std::string						m_szBody;
		 bool								m_bKeepAlive;
	};
}

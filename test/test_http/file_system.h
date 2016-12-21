#pragma once
#include "http_base.h"
#include <map>

namespace http
{
	class CHttpRequest;
	class CHttpResponse;
	class CFileSystem
	{
	public:
		CFileSystem();
		~CFileSystem();

		static CFileSystem* Inst();

		void	handleFile(const std::string& szLogicDir, const std::string& szDir, const CHttpRequest& sRequest, CHttpResponse& sResponse);

	private:
		struct SFileInfo
		{
			std::string szData;
			int64_t		nLastTime;
		};

		std::string							m_szBuf;
		std::map<std::string, SFileInfo>	m_mapFileInfo;
	};
}
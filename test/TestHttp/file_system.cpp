#include "file_system.h"
#include "http_request.h"
#include "http_response.h"
#include <time.h>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace http
{
	CFileSystem::CFileSystem()
	{
		this->m_szBuf.resize(10 * 1024);
	}

	CFileSystem::~CFileSystem()
	{

	}

	CFileSystem* CFileSystem::Inst()
	{
		static CFileSystem s_Inst;

		return &s_Inst;
	}
	
	void CFileSystem::handleFile(const std::string& szLogicDir, const std::string& szDir, const CHttpRequest& sRequest, CHttpResponse& sResponse)
	{
		std::string szName = szDir;
		szName += sRequest.getRequestURI().substr(szLogicDir.size(), sRequest.getRequestURI().size());

		auto iter = this->m_mapFileInfo.find(szName);
		if (iter != this->m_mapFileInfo.end())
		{
			SFileInfo& sFileInfo = iter->second;
			sFileInfo.nLastTime = time(nullptr);
			sResponse.write(sFileInfo.szData.c_str(), (uint32_t)sFileInfo.szData.size());
			return;
		}

		FILE* pFile = fopen(szName.c_str(), "r");
		if (pFile == nullptr)
		{
			sResponse.setStatusCode(eSC_404_NotFound);
			sResponse.setStatusMessage("Not Found");
			sResponse.setKeepAlive(false);

			sResponse.write("404 page not found\n");
			return;
		}

		SFileInfo& sFileInfo = this->m_mapFileInfo[szName];
		while (true)
		{
			size_t nCount = fread(&this->m_szBuf[0], 1, this->m_szBuf.size(), pFile);
			sResponse.write(this->m_szBuf.c_str(), (uint32_t)nCount);
			sFileInfo.szData.append(this->m_szBuf.c_str(), nCount);
			if (nCount != this->m_szBuf.size())
				break;
		}

		sFileInfo.nLastTime = time(nullptr);

		fclose(pFile);
	}
}

#ifdef _WIN32
#pragma warning(pop)
#endif
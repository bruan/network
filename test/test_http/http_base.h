#pragma once

#ifdef _WIN32
#else
#include <unistd.h>
#endif

#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <functional>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace http
{
	enum EMethodType
	{
		eMT_Unknown,
		eMT_Get,
		eMT_Post,
		eMT_Put,
		eMT_Delete
	};

	enum EVersion
	{
		eV_Unknown,
		eV_Http10,
		eV_Http11
	};

	enum EStatusCode
	{
		eSC_Unknown,
		eSC_200_Ok = 200,
		eSC_301_MovedPermanently = 301,
		eSC_400_BadRequest = 400,
		eSC_404_NotFound = 404,
	};

	// 如果目标字符串足以放下格式化后字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
	inline size_t snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...)
	{
		if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
			return -1;

		va_list arg;
		va_start(arg, szFormat);
		size_t ret = vsnprintf(szBuf, nBufSize, szFormat, arg);
		va_end(arg);

		return ret;
	}
	// 如果目标字符串足以放下格式化后字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
	inline size_t vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg)
	{
		if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
			return -1;

#ifdef _WIN32
		// 这里用旧的函数是为了跟linux相同语义
		int32_t nRet = ::_vsnprintf(szBuf, nBufSize, szFormat, arg);
		if (nRet == nBufSize)
		{
			szBuf[nBufSize - 1] = 0;
			--nRet;
		}
		return nRet;
#else
		return ::vsnprintf(szBuf, nBufSize, szFormat, arg);
#endif
	}

	class CHttpRequest;
	class CHttpResponse;
	typedef std::function<void(const CHttpRequest&, CHttpResponse&)> HttpCallback;

}

#ifdef _WIN32
#pragma warning(pop)
#endif
#pragma once

#include <stdint.h>

namespace net
{
	class CNetRecvBuffer
	{
	public:
		CNetRecvBuffer();
		~CNetRecvBuffer();

		bool		init(uint32_t nBufSize);
		void		push(uint32_t nSize);
		void		pop(uint32_t nSize);
		void		resize(uint32_t nSize);
		char*		getFreeBuffer() const;
		char*		getDataBuffer() const;
		uint32_t	getBufferSize() const;
		uint32_t	getDataSize() const;
		uint32_t	getFreeSize() const;

	private:
		char*		m_pBuf;
		uint32_t	m_nBufPos;
		uint32_t	m_nBufSize;
	};

	class CNetSendBuffer;
	class CNetSendBufferBlock
	{
		friend class CNetSendBuffer;

	public:
		CNetSendBufferBlock();
		~CNetSendBufferBlock();

		bool		init(uint32_t nBufSize);
		void		reset();
		void		push(const char* pBuf, uint32_t nSize);
		void		pop(uint32_t nSize);
		char*		getDataBuffer() const;
		uint32_t	getBufferSize() const;
		uint32_t	getDataSize() const;
		uint32_t	getFreeSize() const;

	private:
		char*					m_pBuf;
		uint32_t				m_nDataBegin;
		uint32_t				m_nDataEnd;
		uint32_t				m_pBufSize;
		CNetSendBufferBlock*	m_pNext;
	};

	class CNetSendBuffer
	{
	private:
		uint32_t				m_nBuffBlockSize;
		CNetSendBufferBlock*	m_pHead;
		CNetSendBufferBlock*	m_pTail;

		CNetSendBufferBlock*	m_pNoUse;

	private:
		CNetSendBufferBlock*	getBufferBlock();
		void					putBufferBlock(CNetSendBufferBlock* pBufferBlock);

	public:
		CNetSendBuffer();
		~CNetSendBuffer();

		bool					init(uint32_t nBufSize);
		void					push(const char* pBuf, uint32_t nSize);
		void					pop(uint32_t nSize);
		char*					getDataBuffer(uint32_t& nSize) const;
		uint32_t				getDataSize() const;
		bool					isEmpty() const;
	};
}
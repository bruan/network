#pragma once

#include <stdint.h>
#include <utility>

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
		char*		getWritableBuffer() const;
		char*		getReadableBuffer() const;
		uint32_t	getBufferSize() const;
		uint32_t	getReadableSize() const;
		uint32_t	getWritableSize() const;

	private:
		char*		m_pBuf;
		uint32_t	m_nReadPos;
		uint32_t	m_nWritePos;
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
		char*		getReadableBuffer() const;
		uint32_t	getReadableSize() const;
		uint32_t	getWritableSize() const;
		uint32_t	getBufferSize() const;

	private:
		char*					m_pBuf;
		uint32_t				m_nReadPos;
		uint32_t				m_nWritePos;
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
		char*					getTailData() const;
		uint32_t				getTailDataSize() const;
		uint32_t				getTotalReadableSize() const;
	};
}
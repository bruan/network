#include "stdafx.h"
#include "net_buffer.h"
#include "net_event_loop.h"

namespace net
{
	CNetRecvBuffer::CNetRecvBuffer()
		: m_pBuf(nullptr)
		, m_nBufSize(0)
		, m_nWritePos(0)
		, m_nReadPos(0)
	{
	}

	CNetRecvBuffer::~CNetRecvBuffer()
	{
		delete []this->m_pBuf;
	}

	bool CNetRecvBuffer::init(uint32_t nBufSize)
	{
		DebugAstEx(this->m_pBuf == nullptr, false);

		this->m_nBufSize = nBufSize;
		this->m_pBuf = new char[nBufSize];

		return true;
	}

	void CNetRecvBuffer::resize(uint32_t nSize)
	{
		DebugAst(nSize > this->m_nBufSize);

		char* pNewBuf = new char[nSize];
		memcpy(pNewBuf, this->m_pBuf + this->m_nReadPos, this->m_nWritePos - this->m_nReadPos);
		this->m_nBufSize = nSize;
		this->m_nWritePos = this->m_nWritePos - this->m_nReadPos;
		this->m_nReadPos = 0;
		delete []this->m_pBuf;
		this->m_pBuf = pNewBuf;
	}

	void CNetRecvBuffer::push(uint32_t nSize)
	{
		DebugAst(nSize <= this->getWritableSize());

		this->m_nWritePos += nSize;
	}

	void CNetRecvBuffer::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->getReadableSize());
		this->m_nReadPos += nSize;
		if (this->m_nReadPos > this->m_nBufSize / 2)
		{
			if (this->m_nWritePos - this->m_nReadPos != 0)
				memmove(this->m_pBuf, this->m_pBuf + this->m_nReadPos, this->m_nWritePos - this->m_nReadPos);
			this->m_nWritePos = this->m_nWritePos - this->m_nReadPos;
			this->m_nReadPos = 0;
		}
	}

	char* CNetRecvBuffer::getWritableBuffer() const
	{
		return this->m_pBuf + this->m_nWritePos;
	}

	char* CNetRecvBuffer::getReadableBuffer() const
	{
		return this->m_pBuf + this->m_nReadPos;
	}

	uint32_t CNetRecvBuffer::getBufferSize() const
	{
		return this->m_nBufSize;
	}

	uint32_t CNetRecvBuffer::getReadableSize() const
	{
		return this->m_nWritePos - this->m_nReadPos;
	}

	uint32_t CNetRecvBuffer::getWritableSize() const
	{
		return this->m_nBufSize - this->m_nWritePos;
	}

	CNetSendBufferBlock::CNetSendBufferBlock()
		: m_pBuf(nullptr)
		, m_pNext(nullptr)
		, m_nReadPos(0)
		, m_nWritePos(0)
		, m_pBufSize(0)
	{
	}

	CNetSendBufferBlock::~CNetSendBufferBlock()
	{
		delete []this->m_pBuf;
	}

	bool CNetSendBufferBlock::init(uint32_t nBufSize)
	{
		DebugAstEx(this->m_pBuf == nullptr, false);

		this->m_pBuf = new char[nBufSize];
		this->m_pBufSize = nBufSize;
		this->reset();

		return true;
	}

	void CNetSendBufferBlock::reset()
	{
		this->m_nWritePos = 0;
		this->m_nReadPos = 0;
		this->m_pNext = nullptr;
	}

	void CNetSendBufferBlock::push(const char* pBuf, uint32_t nSize)
	{
		DebugAst(nSize <= this->getWritableSize());

		memcpy(this->m_pBuf + this->m_nWritePos, pBuf, nSize);
		this->m_nWritePos += nSize;
	}

	void CNetSendBufferBlock::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->getReadableSize());

		this->m_nReadPos += nSize;
	}

	char* CNetSendBufferBlock::getReadableBuffer() const
	{
		return this->m_pBuf + this->m_nReadPos;
	}

	uint32_t CNetSendBufferBlock::getBufferSize() const
	{
		return this->m_pBufSize;
	}

	uint32_t CNetSendBufferBlock::getReadableSize() const
	{
		return this->m_nWritePos - this->m_nReadPos;
	}

	uint32_t CNetSendBufferBlock::getWritableSize() const
	{
		return this->m_pBufSize - this->m_nWritePos;
	}

	CNetSendBuffer::CNetSendBuffer()
		: m_pHead(nullptr)
		, m_pTail(nullptr)
		, m_pNoUse(nullptr)
		, m_nBuffBlockSize(0)
	{
	}

	CNetSendBuffer::~CNetSendBuffer()
	{
		for (CNetSendBufferBlock* pSendBufferBlock = this->m_pHead; pSendBufferBlock != nullptr;)
		{
			CNetSendBufferBlock* pDelSendBufferBlock = pSendBufferBlock;
			pSendBufferBlock = pSendBufferBlock->m_pNext;
			delete pDelSendBufferBlock;
		}

		for (CNetSendBufferBlock* pSendBufferBlock = this->m_pNoUse; pSendBufferBlock != nullptr;)
		{
			CNetSendBufferBlock* pDelSendBufferBlock = pSendBufferBlock;
			pSendBufferBlock = pSendBufferBlock->m_pNext;
			delete pDelSendBufferBlock;
		}
	}

	bool CNetSendBuffer::init(uint32_t nBufSize)
	{
		this->m_nBuffBlockSize = nBufSize;
		this->m_pHead = this->m_pTail = nullptr;

		// 先创建一个buf
		this->m_pNoUse = new CNetSendBufferBlock();
		if (!this->m_pNoUse->init(this->m_nBuffBlockSize))
		{
			delete this->m_pNoUse;
			return false;
		}

		return true;
	}

	CNetSendBufferBlock* CNetSendBuffer::getBufferBlock()
	{
		if (this->m_pNoUse == nullptr)
		{
			CNetSendBufferBlock* pBufferBlock = new CNetSendBufferBlock();
			if (!pBufferBlock->init(this->m_nBuffBlockSize))
			{
				delete pBufferBlock;
				return nullptr;
			}

			return pBufferBlock;
		}
		CNetSendBufferBlock* pBufferBlock = this->m_pNoUse;
		this->m_pNoUse = this->m_pNoUse->m_pNext;
		pBufferBlock->reset();

		return pBufferBlock;
	}

	void CNetSendBuffer::putBufferBlock(CNetSendBufferBlock* pBufferBlock)
	{
		DebugAst(pBufferBlock != nullptr);

		pBufferBlock->m_pNext = this->m_pNoUse;
		this->m_pNoUse = pBufferBlock;
	}

	void CNetSendBuffer::push(const char* pBuf, uint32_t nSize)
	{
		if (nullptr == this->m_pHead)
		{
			this->m_pHead = this->getBufferBlock();
			this->m_pTail = this->m_pHead;
		}
		while (true)
		{
			if (this->m_pHead->getWritableSize() >= nSize)
			{
				this->m_pHead->push(pBuf, nSize);
				break;
			}
			else
			{
				uint32_t nFreeSize = this->m_pHead->getWritableSize();
				nSize -= nFreeSize;
				this->m_pHead->push(pBuf, this->m_pHead->getWritableSize());
				pBuf += nFreeSize;
				CNetSendBufferBlock* pNewSendBufferBlock = this->getBufferBlock();
				pNewSendBufferBlock->m_pNext = this->m_pHead;
				this->m_pHead = pNewSendBufferBlock;
			}
		}
	}

	void CNetSendBuffer::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->m_pTail->getReadableSize());

		this->m_pTail->pop(nSize);
		if (this->m_pTail->getReadableSize() == 0)
		{
			this->putBufferBlock(this->m_pTail);
			if (this->m_pTail != this->m_pHead)
			{
				for (CNetSendBufferBlock* pSendBufferBlock = this->m_pHead; pSendBufferBlock != nullptr; pSendBufferBlock = pSendBufferBlock->m_pNext)
				{
					if (pSendBufferBlock->m_pNext == this->m_pTail)
					{
						pSendBufferBlock->m_pNext = nullptr;
						this->m_pTail = pSendBufferBlock;
						break;
					}
				}
			}
			else
			{
				this->m_pHead = this->m_pTail = nullptr;
			}
		}
	}

	char* CNetSendBuffer::getTailData() const
	{
		if (nullptr == this->m_pTail)
			return nullptr;

		return this->m_pTail->getReadableBuffer();
	}

	uint32_t CNetSendBuffer::getTailDataSize() const
	{
		if (nullptr == this->m_pTail)
			return 0;

		return this->m_pTail->getReadableSize();
	}

	uint32_t CNetSendBuffer::getTotalReadableSize() const
	{
		uint32_t nDataSize = 0;
		for (CNetSendBufferBlock* pSendBufferBlock = this->m_pHead; pSendBufferBlock != nullptr; pSendBufferBlock = pSendBufferBlock->m_pNext)
		{
			nDataSize += pSendBufferBlock->getReadableSize();
		}

		return nDataSize;
	}
}
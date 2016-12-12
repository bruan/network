#include "stdafx.h"
#include "net_buffer.h"
#include "net_event_loop.h"


namespace base
{
	CNetRecvBuffer::CNetRecvBuffer()
		: m_pBuf(nullptr)
		, m_nBufSize(0)
		, m_nBufPos(0)
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
		memcpy(pNewBuf, this->m_pBuf, this->m_nBufPos);
		this->m_nBufSize = nSize;
		delete []this->m_pBuf;
		this->m_pBuf = pNewBuf;
	}

	void CNetRecvBuffer::push(uint32_t nSize)
	{
		DebugAst(nSize <= this->getFreeSize());

		this->m_nBufPos += nSize;
	}

	void CNetRecvBuffer::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->getDataSize());
		this->m_nBufPos = (this->m_nBufPos - nSize);
		memmove(this->m_pBuf, this->m_pBuf + nSize, this->m_nBufPos);
	}

	char* CNetRecvBuffer::getFreeBuffer() const
	{
		return this->m_pBuf + this->m_nBufPos;
	}

	char* CNetRecvBuffer::getDataBuffer() const
	{
		return this->m_pBuf;
	}

	uint32_t CNetRecvBuffer::getBufferSize() const
	{
		return this->m_nBufSize;
	}

	uint32_t CNetRecvBuffer::getDataSize() const
	{
		return this->m_nBufPos;
	}

	uint32_t CNetRecvBuffer::getFreeSize() const
	{
		return this->m_nBufSize - this->m_nBufPos;
	}

	CNetSendBufferBlock::CNetSendBufferBlock()
		: m_pBuf(nullptr)
		, m_pNext(nullptr)
		, m_nDataBegin(0)
		, m_nDataEnd(0)
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
		this->m_nDataEnd = 0;
		this->m_nDataBegin = 0;
		this->m_pNext = nullptr;
	}

	void CNetSendBufferBlock::push(const char* pBuf, uint32_t nSize)
	{
		DebugAst(nSize <= this->getFreeSize());

		memcpy(this->m_pBuf + this->m_nDataEnd, pBuf, nSize);
		this->m_nDataEnd += nSize;
	}

	void CNetSendBufferBlock::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->getDataSize());

		this->m_nDataBegin += nSize;
	}

	char* CNetSendBufferBlock::getDataBuffer() const
	{
		return this->m_pBuf + this->m_nDataBegin;
	}

	uint32_t CNetSendBufferBlock::getBufferSize() const
	{
		return this->m_pBufSize;
	}

	uint32_t CNetSendBufferBlock::getDataSize() const
	{
		return this->m_nDataEnd - this->m_nDataBegin;
	}

	uint32_t CNetSendBufferBlock::getFreeSize() const
	{
		return this->m_pBufSize - this->m_nDataEnd;
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
			if (this->m_pHead->getFreeSize() >= nSize)
			{
				this->m_pHead->push(pBuf, nSize);
				break;
			}
			else
			{
				uint32_t nFreeSize = this->m_pHead->getFreeSize();
				nSize -= nFreeSize;
				this->m_pHead->push(pBuf, this->m_pHead->getFreeSize());
				pBuf += nFreeSize;
				CNetSendBufferBlock* pNewSendBufferBlock = this->getBufferBlock();
				pNewSendBufferBlock->m_pNext = this->m_pHead;
				this->m_pHead = pNewSendBufferBlock;
			}
		}
	}

	void CNetSendBuffer::pop(uint32_t nSize)
	{
		DebugAst(nSize <= this->m_pTail->getDataSize());

		this->m_pTail->pop(nSize);
		if (this->m_pTail->getDataSize() == 0)
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

	char* CNetSendBuffer::getDataBuffer(uint32_t& nSize) const
	{
		if (nullptr == this->m_pTail)
		{
			nSize = 0;
			return nullptr;
		}

		nSize = this->m_pTail->getDataSize();

		return this->m_pTail->getDataBuffer();
	}

	uint32_t CNetSendBuffer::getDataSize() const
	{
		uint32_t nDataSize = 0;
		for (CNetSendBufferBlock* pSendBufferBlock = this->m_pHead; pSendBufferBlock != nullptr; pSendBufferBlock = pSendBufferBlock->m_pNext)
		{
			nDataSize += pSendBufferBlock->getDataSize();
		}

		return nDataSize;
	}

	bool CNetSendBuffer::isEmpty() const
	{
		uint32_t nSize = 0;
		this->getDataBuffer(nSize);
		return nSize == 0;
	}
}
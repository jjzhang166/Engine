#ifndef		__ENGINE_POOL_H_INCLUDED__
#define		__ENGINE_POOL_H_INCLUDED__

#include	<cstdlib>
#include	<cstring>
#include	<list>

/**
 * Non-threadsafe Object Pool.
 **/
template<typename T>
class Pool {
	struct Chunk {
		Chunk *	pNext;
		bool	bUsed;
		T *		pData;

		inline void Init() {
			pNext = nullptr;
			bUsed = false;
			pData = (T *)((char *)this + sizeof(Chunk));
		}
	};

public:
	Pool(size_t nStep) : _nStep(nStep), _pHead(new Chunk) {
		_pHead->Init();
		__Extend();
	}

	virtual ~Pool() {
		Clear();
		for (auto pMem : _lMem) delete[] pMem;
		_lMem.clear();
	}

	/**
	 * Alloc a object with parameters. You should call Free() to release resources.
	 **/
	template<typename ... Args>
	T *	Alloc(Args ... args) {
		if (!_pHead->pNext && !__Extend()) return nullptr;

		Chunk * pNode = _pHead->pNext;
		_pHead->pNext = _pHead->pNext->pNext;
		pNode->bUsed = true;
		return new (pNode->pData) T(args...);
	}

	/**
	 * Free resources used by this object.
	 **/
	void Free(T * pObj) {
		if (!pObj) return;
		pObj->~T();
		memset(pObj, 0, sizeof(T));

		Chunk * pNode = (Chunk *)((char *)pObj - sizeof(Chunk));
		pNode->bUsed = false;
		pNode->pNext = _pHead->pNext;
		_pHead->pNext = pNode;
	}

	/**
	 * Clear all current objects in this pool.
	 **/
	void Clear() {
		size_t nBlock = sizeof(Chunk) + sizeof(T);
		for (auto pMem : _lMem) {
			for (size_t nLoop = 0; nLoop < _nStep; ++nLoop) {
				Chunk * pCur = (Chunk *)(pMem + nLoop * nBlock);
				if (pCur->bUsed) {
					pCur->bUsed = false;
					pCur->pData->~T();
					memset(pCur->pData, 0, sizeof(T));

					pCur->pNext = _pHead->pNext;
					_pHead->pNext = pCur;
				}
			}
		} 
	}

private:
	bool __Extend() {
		size_t nBlock = sizeof(Chunk) + sizeof(T);
		char * pMem = new char[nBlock * _nStep];
		if (!pMem) return false;

		char * pCur = pMem;
		for (size_t nLoop = 0; nLoop < _nStep; ++nLoop, pCur += nBlock) {
			Chunk * pNode = (Chunk *)(pCur);
			pNode->Init();
			pNode->pNext = _pHead->pNext;
			_pHead->pNext = pNode;
		}

		_lMem.push_back(pMem);
		return true;
	}

private:
	size_t				_nStep;
	Chunk *				_pHead;
	std::list<char *>	_lMem;
};

#endif//!	__ENGINE_POOL_H_INCLUDED__
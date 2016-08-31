#include	"FastBuffer.h"
#include	<cstdlib>
#include	<cstring>

FastBuffer::FastBuffer(size_t nCapacity)
	: _nCur(0)
	, _nCapacity(nCapacity) {
	_pMem[0] = new char[nCapacity];
	_pMem[1] = new char[nCapacity];

	memset(_pMem[0], 0, nCapacity);
	memset(_pMem[1], 0, nCapacity);
}

FastBuffer::~FastBuffer() {
	delete[] _pMem[0];
	delete[] _pMem[1];
}

bool FastBuffer::Write(void * pData, size_t nSize) {
	char * pMem = _pMem[_nCur.load()];
	size_t * pSize = (size_t *)pMem;
	size_t nWrited = *pSize;
	if (nWrited + nSize > _nCapacity) return false;
	*pSize = nWrited + nSize;
	memcpy(pMem + sizeof(size_t) + nWrited, pData, nSize);
	return true;
}

bool FastBuffer::Read(char ** pData, size_t & nSize) {
	int nPrev = _nCur.load();
	memset(_pMem[1 - nPrev], 0, _nCapacity);
	_nCur.exchange(1 - nPrev);
	nSize = *((size_t *)(_pMem[nPrev]));
	*pData = _pMem[nPrev] + sizeof(size_t);
	return nSize > 0;
}
#ifndef		__ENGINE_FASTBUFFER_H_INCLUDED__
#define		__ENGINE_FASTBUFFER_H_INCLUDED__

#include	<atomic>
#include	<cstdlib>

/**
 * Non-block buffer. Useful tool in two threads(One-read, One-write)
 */
class FastBuffer {
public:
	FastBuffer(size_t nCapacity);
	virtual ~FastBuffer();

	/**
	 * Write data into this buffer.
	 *
	 * \param	pData	Pointer to data.
	 * \param	nSize	Size of data in bytes.
	 * \return	True for written successfully.
	 */
	bool Write(void * pData, size_t nSize);

	/**
	 * Read all data in this buffer. This will swap buffers.
	 *
	 * \param	pData	Out pointer to hold data start.
	 * \param	nSize	Total size of data in this buffer.
	 * \returns	Is there at least one byte data in buffer?
	 */
	bool Read(char ** pData, size_t & nSize);

private:
	std::atomic<int>	_nCur;
	size_t				_nCapacity;
	char *				_pMem[2];
};

#endif//!	__ENGINE_FASTBUFFER_H_INCLUDED__

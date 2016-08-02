#ifndef		__ENGINE_CRYPTO_H_INCLUDED__
#define		__ENGINE_CRYPTO_H_INCLUDED__

#include	<cstdint>
#include	<string>

/**
 * Calculate a data's CRC32.
 *
 * \param	pData	Pointer to data to be calculated.
 * \param	nSize	Size of data in bytes.
 * \param	nAppend	If this data is subset of a whole buffer, you should pass the result of CalcCRC before.
 * \return	CRC32 result.
 **/
extern uint32_t CalcCRC(const char * pData, size_t nSize, uint32_t nAppend = 0);

/**
 * Data hash using BKDRHash
 *
 * \param	pData	Pointer to data to be calculated.
 * \param	nSize	Size of data in bytes.
 * \return	Hash result.
 **/
extern uint32_t	CalcHash(const char * pData, size_t nSize);

/**
 * MD5 tools.
 **/
class MD5 {
public:
	MD5() { Reset(); }
	MD5(const char * pData, size_t nSize) { Reset(); Update(pData, nSize); }
	MD5(const std::string & sData) { Reset(); Update(sData); }
	MD5(MD5 & r) { Copy(r.Digest()); }

	bool operator==(const MD5 & r);
	bool operator!=(const MD5 & r);

	void	Update(const char * pData, size_t nSize);
	void	Update(const std::string & sData);

	const uint8_t *	Digest();
	std::string		ToString();
	void			Copy(const uint8_t * p);
	void			Reset();

private:
	void	__Transform();
	void	__Final();

private:
	bool		_bFinished;
	size_t		_nSize;
	uint8_t		_pDigest[16];
	uint8_t		_pTail[64];
	uint32_t	_pState[4];
};

#endif//!	__ENGINE_CRYPTO_H_INCLUDED__
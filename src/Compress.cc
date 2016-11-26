#include	<Compress.h>
#include	<cstring>
#include	"miniz/miniz.h"

Zip::Zip() : _pZip(new mz_zip_archive), _bValid(false), _vItems() {
	memset(_pZip, 0, sizeof(mz_zip_archive));
}

Zip::~Zip() {
	Close();
	delete _pZip;
}

size_t Zip::CalcCompressed(size_t nSize) {
	return (size_t)mz_compressBound((mz_ulong)nSize);
}

int Zip::Compress(char * pDst, size_t & nOutSize, const char * pSrc, size_t nSrcSize) {
	mz_ulong nDst = (mz_ulong)nOutSize;
	int nRet = mz_compress((unsigned char *)pDst, &nDst, (const unsigned char *)pSrc, (mz_ulong)nSrcSize);
	nOutSize = (size_t)nDst;
	return nRet;
}

int Zip::Uncompress(char * pDst, size_t & nOutSize, const char * pSrc, size_t nSrcSize) {
	mz_ulong nDst = (mz_ulong)nOutSize;
	int nRet = mz_uncompress((unsigned char *)pDst, &nDst, (const unsigned char *)pSrc, (mz_ulong)nSrcSize);
	nOutSize = (size_t)nDst;
	return nRet;
}

bool Zip::LoadFile(const char * pFile) {
	Close();

	if (!mz_zip_reader_init_file(_pZip, pFile, 0)) return false;

	size_t nCount = mz_zip_reader_get_num_files(_pZip);
	for (size_t nIdx = 0; nIdx < nCount; ++nIdx) {
		mz_zip_archive_file_stat iStat;
		if (!mz_zip_reader_file_stat(_pZip, (mz_uint)nIdx, &iStat)) continue;
		Item iInfo;
		::memcpy(iInfo.sName, iStat.m_filename, 256);
		iInfo.bDir = mz_zip_reader_is_file_a_directory(_pZip, (mz_uint)nIdx) == 1;
		iInfo.nSize = (size_t)iStat.m_uncomp_size;
		_vItems.push_back(iInfo);
	}

	_bValid = true;
	return true;
}

bool Zip::LoadMemory(const char * pData, size_t nSize) {
	Close();

	if (!mz_zip_reader_init_mem(_pZip, pData, nSize, 0)) return false;

	size_t nCount = mz_zip_reader_get_num_files(_pZip);
	for (size_t nIdx = 0; nIdx < nCount; ++nIdx) {
		mz_zip_archive_file_stat iStat;
		if (!mz_zip_reader_file_stat(_pZip, (mz_uint)nIdx, &iStat)) continue;
		Item iInfo;
		::memcpy(iInfo.sName, iStat.m_filename, 256);
		iInfo.bDir = mz_zip_reader_is_file_a_directory(_pZip, (mz_uint)nIdx) == 1;
		iInfo.nSize = (size_t)iStat.m_uncomp_size;
		_vItems.push_back(iInfo);
	}

	_bValid = true;
	return true;
}

Zip::Item * Zip::Get(int nIdx) {
	if (nIdx < 0 || nIdx > (int)_vItems.size()) return nullptr;
	return &_vItems[nIdx];
}

Zip::Item * Zip::Get(const char * sName) {
	for (auto it = _vItems.begin(); it != _vItems.end(); ++it) {
		if (strncmp(sName, it->sName, 256) == 0) return &(*it);
	}
	return nullptr;
}

void * Zip::Extra(int nIdx, size_t & nOutSize) {
	if (!_bValid || nIdx < 0 || nIdx > (int)_vItems.size()) return nullptr;
	return mz_zip_reader_extract_to_heap(_pZip, nIdx, &nOutSize, 0);
}

void * Zip::Extra(const char * sName, size_t & nOutSize) {
	if (!_bValid) return nullptr;
	return mz_zip_reader_extract_file_to_heap(_pZip, sName, &nOutSize, 0);
}

void Zip::Close() {
	if (_bValid) {
		mz_zip_reader_end(_pZip);
		memset(_pZip, 0, sizeof(mz_zip_archive));
		_bValid = false;
	}
	_vItems.clear();
}


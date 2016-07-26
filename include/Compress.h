#ifndef		__ENGINE_COMPRESS_H_INCLUDED__
#define		__ENGINE_COMPRESS_H_INCLUDED__

#include	<cstdlib>
#include	<vector>

namespace EZip {
	enum State {
		Success		= 0,
		StreamEnd	= 1,
		NeedDict	= 2,
		ErrorNo		= -1,
		StreamError	= -2,
		DataError	= -3,
		MemError	= -4,
		BufError	= -5,
		VerError	= -6
	};
}

class Zip {
public:
	struct Item {
		char	sName[256];
		bool	bDir;
		size_t	nSize;
	};

	typedef std::vector<Item>	ItemList;

	class Iterator {
	public:
		Iterator(ItemList & r) : _itCur(r.begin()), _itEnd(r.end()) {}

		inline			operator bool() const { return _itCur != _itEnd; }
		inline void		operator++() { _itCur++; }
		inline Item *	Data() { return &(*_itCur); }

	private:
		ItemList::iterator	_itCur;
		ItemList::iterator	_itEnd;
	};

public:
	Zip();
	virtual ~Zip();

	static size_t	CalcCompressed(size_t nSize);
	static int		Compress(char * pDst, size_t & nOutSize, const char * pSrc, size_t nSrcSize);
	static int		Uncompress(char * pDst, size_t & nOutSize, const char * pSrc, size_t nSrcSize);

	bool	LoadFile(const char * pFile);
	bool	LoadMemory(const char * pData, size_t nSize);

	int		Count() { return (int)_vItems.size(); }
	Item *	Get(int nIdx);
	Item *	Get(const char * sName);
	void *	Extra(int nIdx, size_t & nOutSize);
	void *	Extra(const char * sName, size_t & nOutSize);
	void	Close();

	Iterator	GetIterator() { return Iterator(_vItems); }

private:
	struct mz_zip_archive_tag *	_pZip;
	bool						_bValid;
	ItemList					_vItems;
};

#endif//!	__ENGINE_COMPRESS_H_INCLUDED__
#include	<Crypto.h>
#include	<cstring>

static const uint32_t CRCTABLE[] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

uint32_t CalcCRC(const char * pData, size_t nSize, uint32_t nAppend) {
	if (!pData || nSize <= 0) return nAppend;

	uint32_t nValue = nAppend ^ 0xFFFFFFFF;

	for (size_t nPos = 0; nPos < nSize; ++nPos) {
		nValue = (nValue >> 8) ^ CRCTABLE[(nValue ^ pData[nPos]) & 0xFF];
	}

	return nValue ^ 0xFFFFFFFF;
}

uint32_t CalcHash(const char * pData, size_t nSize) {
	uint32_t nHash = 0;
	for (size_t i = 0; i < nSize; ++i) nHash = nHash * 131 + pData[i];
	return nHash & 0x7FFFFFFF;
}

static const uint32_t MD5KEY[] = {
	0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
	0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
	0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
	0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
	0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
	0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
	0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
	0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

static const uint8_t MD5DATA[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
	0x1, 0x6, 0xB, 0x0, 0x5, 0xA, 0xF, 0x4,
	0x9, 0xE, 0x3, 0x8, 0xD, 0x2, 0x7, 0xC,
	0x5, 0x8, 0xB, 0xE, 0x1, 0x4, 0x7, 0xA,
	0xD, 0x0, 0x3, 0x6, 0x9, 0xC, 0xF, 0x2,
	0x0, 0x7, 0xE, 0x5, 0xC, 0x3, 0xA, 0x1,
	0x8, 0xF, 0x6, 0xD, 0x4, 0xB, 0x2, 0x9,
};

static const uint8_t MD5SHIFT[] = {
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

bool MD5::operator==(const MD5 & r) {
	__Final();
	return memcmp(_pDigest, r._pDigest, 16) == 0;
}

bool MD5::operator!=(const MD5 & r) {
	__Final();
	return memcmp(_pDigest, r._pDigest, 16) != 0;
}

void MD5::Update(const std::string & sData) {
	Update(sData.data(), sData.size());
}

void MD5::Update(const char * pData, size_t nSize) {
	if (_bFinished) Reset();

	size_t nTail = _nSize % 64;
	size_t nRead = 0;
	_nSize += nSize;

	if (nTail > 0) {
		if (nSize < 64 - nTail) {
			memcpy(_pTail + nTail, pData, nSize);
			return;
		} else {
			memcpy(_pTail + nTail, pData, 64 - nTail);
			__Transform();
			nRead = 64 - nTail;
		}
	}	

	size_t nLoop = (nSize - nRead) / 64;
	for (size_t i = 0; i < nLoop; ++i) {
		memcpy(_pTail, pData + nRead, 64);
		nRead += 64;
		__Transform();
	}

	size_t nLeft = _nSize % 64;
	if (nLeft > 0) memcpy(_pTail, pData + nRead, nLeft);
}

const uint8_t * MD5::Digest() {
	__Final();
	return _pDigest;
}

std::string MD5::ToString() {
	const uint8_t * p = Digest();

	char pHex[64];
	snprintf(pHex, 64, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\0",
		p[0], p[1], p[2], p[3],	p[4], p[5], p[6], p[7],
		p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
	return std::string(pHex);
}

void MD5::Copy(const uint8_t * p) {
	Reset();
	_bFinished = true;
	memcpy(_pDigest, p, 16);
}

void MD5::Reset() {
	_bFinished = false;
	_nSize = 0;

	memset(_pDigest, 0, 16);
	memset(_pTail, 0, 64);

	_pState[0] = 0x67452301;
	_pState[1] = 0xEFCDAB89;
	_pState[2] = 0x98BADCFE;
	_pState[3] = 0x10325476;
}

void MD5::__Transform() {
	uint32_t pWord[16] = { 0 };
	
	for (int i = 0; i < 16; ++i) {
		int nStart = i * 4;
		pWord[i] = (uint32_t)_pTail[nStart] | 
			((uint32_t)_pTail[nStart + 1] << 8) |
			((uint32_t)_pTail[nStart + 2] << 16) |
			((uint32_t)_pTail[nStart + 3] << 24);
	}

	uint32_t nA = _pState[0];
	uint32_t nB = _pState[1];
	uint32_t nC = _pState[2];
	uint32_t nD = _pState[3];
	uint32_t nT = 0;

	for (int i = 0; i < 64; ++i) {
		if (i <= 15) {
			nT = (nB & nC) | ((~nB) & nD);
		} else if (i <= 31) {
			nT = (nD & nB) | ((~nD) & nC);
		} else if (i <= 47) {
			nT = nB ^ nC ^ nD;
		} else {
			nT = nC ^ (nB | (~nD));
		}

		uint32_t nRotate = (nA + nT + MD5KEY[i] + pWord[MD5DATA[i]]);

		nT = nD;
		nD = nC;
		nC = nB;
		nB = nB + ((nRotate << MD5SHIFT[i]) | (nRotate >> (32 - MD5SHIFT[i])));
		nA = nT;
	}

	_pState[0] += nA;
	_pState[1] += nB;
	_pState[2] += nC;
	_pState[3] += nD;

	memset(_pTail, 0, 64);
}

void MD5::__Final() {
	if (_bFinished) return;
	_bFinished = true;

	/// Append bit '1' to end of data.
	size_t nTail = _nSize % 64;
	_pTail[nTail] = 0x80;
	if (nTail + 1 > 56) __Transform();

	/// Append data size.
	uint64_t nSize = (uint64_t)_nSize * 8;
	for (int n = 8; n > 0; --n) 	_pTail[64 - n] = (nSize >> ((8 - n) * 8)) & 0xFF;
	__Transform();

	/// Dump result into digest.
	for (int i = 0, j = 0; i < 16; i += 4, j++) {
		_pDigest[i] = _pState[j] & 0xFF;
		_pDigest[i + 1] = (_pState[j] >> 8) & 0xFF;
		_pDigest[i + 2] = (_pState[j] >> 16) & 0xFF;
		_pDigest[i + 3] = (_pState[j] >> 24) & 0xFF;
	}
}

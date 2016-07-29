#include	<CsvFile.h>
#include	<Guard.h>
#include	<cstdio>
#include	<cstring>

#define		CSV_LINE_MAX	8192

bool CsvFile::Load(const std::string & sFile, int nHeader /* = 0 */, char nDelimiter /* = '\t' */) {
	if (sFile.empty()) return false;

	FILE * pFile = nullptr;
	Guard iAuto([&](){ if (pFile) fclose(pFile); });

#if defined(_WIN32)
	if (fopen_s(&pFile, sFile.c_str(), "r") != 0) return false;
#else
	if ((pFile = fopen(sFile.c_str(), "r")) == NULL) return false;
#endif

	_vHeader.clear();
	_vData.clear();

	unsigned char pBomb[3] = { 0, 0, 0 };
	int nCount = fread(pBomb, 1, 3, pFile);
	if (nCount < 3 || !(pBomb[0] == 0xEF && pBomb[1] == 0xBB && pBomb[2] == 0xBF))
		fseek(pFile, 0, SEEK_SET);

	char pLine[CSV_LINE_MAX] = { 0 };
	char pDelim[2] = { nDelimiter, 0 };

	/** Ignore all data before header line */
	for (int i = 0; i < nHeader; ++i) {
		fgets(pLine, CSV_LINE_MAX, pFile);
		if (feof(pFile)) return true;
	}

	/** Read headers */
	memset(pLine, 0, CSV_LINE_MAX);
	fgets(pLine, CSV_LINE_MAX, pFile);
	_vHeader = Split(Trim(pLine, "\r\n"), pDelim, false);

	/** Read datas */
	while (!feof(pFile)) {
		memset(pLine, 0, CSV_LINE_MAX);
		fgets(pLine, CSV_LINE_MAX, pFile);

		std::string sRecord = Trim(pLine, "\r\n");
		if (sRecord.find_first_not_of(pDelim) == std::string::npos) continue;

		std::vector<std::string> vRecord = Split(sRecord, pDelim, false);
		_vData.push_back(vRecord);
	}

	return true;
}

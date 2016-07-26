#include	<Logger.h>
#include	<DateTime.h>
#include	<Path.h>

#include	<cstdarg>
#include	<cstdio>
#include	<cstdlib>
#include	<cstring>
#include	<memory>

#if defined(_WIN32)
#	include		<Windows.h>
#endif

using namespace std;

static const char * __LOG_ERR = "ERR";
static const char * __LOG_WRN = "WRN";
static const char * __LOG_INF = "INF";
static const char * __LOG_DBG = "DBG";

Logger::Logger()
	: _pFile(nullptr)
	, _pBuf(new char[1024 * 1024])
	, _sName("Main")
	, _sPath("logs")
	, _nMaxSize(1024 * 1024 * 4)
	, _emMaxLevel(ELog::Info)
	, _bStdOut(true)
	, _nLastDay(-1)
	, _nWrited(0) {}

Logger::~Logger() {
	if (_pFile) fclose(_pFile);
	if (_pBuf) delete[] _pBuf;
}

Logger & Logger::Instance() {
	static mutex _iLock;
	static unique_ptr<Logger> _iIns;

	{
		unique_lock<mutex> _(_iLock);
		if (!_iIns.get()) _iIns.reset(new Logger);
		return *(_iIns.get());
	}	
}

void Logger::Init(const string & sName, const string & sPath, size_t nMaxSize, ELog::Level emMaxLevel, bool bStdOut) {
	_sName = sName;
	_sPath = sPath;
	_nMaxSize = nMaxSize;
	_emMaxLevel = emMaxLevel;
	_bStdOut = bStdOut;
}

void Logger::Log(ELog::Level emLevel, const char * pFile, int nLine, const char * pFmt, ...) {
	if (emLevel > _emMaxLevel) return;

	const char * pType = __LOG_INF;
	switch (emLevel) {
	case ELog::Debug: pType = __LOG_DBG; break;
	case ELog::Info: pType = __LOG_INF; break;
	case ELog::Warning: pType = __LOG_WRN; break;
	case ELog::Error: pType = __LOG_ERR; break;
	}

	std::unique_lock<std::mutex> iAuto(_iLock);

	DateTime iCur;	
	if (_nLastDay != iCur.nDay || _nWrited >= _nMaxSize) __Create(&iCur);
	if (!_pFile) return;

	memset(_pBuf, 0, 1024 * 1024);

	int nOffset = snprintf(_pBuf, 1024 * 1024, "[%04d-%02d-%02d %02d:%02d:%02d.%03d][%s][%s@%d] ", 
		iCur.nYear, iCur.nMonth, iCur.nDay, iCur.nHour, iCur.nMin, iCur.nSec, iCur.nMSec,
		pType, pFile, nLine);
	if (nOffset <= 0) return;

	va_list args;
	va_start(args, pFmt);
	vsnprintf(_pBuf + nOffset, 1024 * 1024 - nOffset, pFmt, args);
	va_end(args);

	_nWrited += fprintf(_pFile, "%s\n", _pBuf);
	fflush(_pFile);

	if (!_bStdOut) return;

#if defined(_WIN32)
	static HANDLE __STD_OUT = GetStdHandle(STD_OUTPUT_HANDLE);

	if (emLevel == ELog::Debug) {
		SetConsoleTextAttribute(__STD_OUT, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	} else if (emLevel == ELog::Warning) {
		SetConsoleTextAttribute(__STD_OUT, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
	} else if (emLevel == ELog::Error) {
		SetConsoleTextAttribute(__STD_OUT, FOREGROUND_RED | FOREGROUND_INTENSITY);
	}

	printf("%s\n", _pBuf);

	if (emLevel != ELog::Info)
		SetConsoleTextAttribute(__STD_OUT, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#else
	if (emLevel == ELog::Debug) {
		printf("%s\n", _pBuf);
	} else if (emLevel == ELog::Warning) {
		printf("\033[33m%s\n\033[0m", _pBuf);
	} else if (emLevel == ELog::Error) {
		printf("\033[31m%s\n\033[0m", _pBuf);
	} else {
		printf("\033[32m%s\n\033[0m", _pBuf);
	}
#endif
}

void Logger::__Create(DateTime * pCreate) {
	std::string sPath(512, '\0');
	std::string sFile(512, '\0');

	snprintf((char *)sPath.c_str(), 512, "%s/%04d%02d%02d", _sPath.c_str(), pCreate->nYear, pCreate->nMonth, pCreate->nDay);
	snprintf((char *)sFile.c_str(), 512, "%s/%s_%02d_%02d_%02d.%03d.log", sPath.c_str(), _sName.c_str(), pCreate->nHour, pCreate->nMin, pCreate->nSec, pCreate->nMSec);

	if (!Path::Exists(_sPath) && !Path::Create(_sPath)) return;
	if (!Path::Exists(sPath) && !Path::Create(sPath)) return;
	if (!(_pFile = fopen(sFile.c_str(), "w"))) return;

	_nWrited	= 0;
	_nLastDay	= pCreate->nDay;
}
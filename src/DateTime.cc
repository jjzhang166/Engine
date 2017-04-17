#include	<DateTime.h>
#include	<ctime>

#if defined(_WIN32)
#	include		<Windows.h>
#endif

#if defined(_WIN32)
uint64_t Tick() {
	static bool		bInited = false;
	static double	dFreq	= 0;

	if (!bInited) {
		LARGE_INTEGER iPerfFreq;
		if (QueryPerformanceFrequency(&iPerfFreq)) {
			dFreq = 1.0 / iPerfFreq.QuadPart;
		} else {
			dFreq = 0;
		}

		bInited = true;
	}

	LARGE_INTEGER iCounter;
	if (!QueryPerformanceCounter(&iCounter)) return 0;
	return (uint64_t)((double)iCounter.QuadPart * dFreq * 1000);
}
#else
uint64_t Tick() {
	struct timespec iClock;
	clock_gettime(CLOCK_MONOTONIC, &iClock);
	return (uint64_t)iClock.tv_sec * 1000 + iClock.tv_nsec / 1000000;
}
#endif

DateTime::DateTime() {	
#if defined(_WIN32)
	SYSTEMTIME iInfo;
	GetLocalTime(&iInfo);

	nYear		= iInfo.wYear;
	nMonth		= iInfo.wMonth;
	nDay	 	= iInfo.wDay;
	nWeek		= iInfo.wDayOfWeek;
	nHour		= iInfo.wHour;
	nMin		= iInfo.wMinute;
	nSec		= iInfo.wSecond;
	nMSec		= iInfo.wMilliseconds;
#else
	struct tm iInfo;
	struct timespec iDetail;

	clock_gettime(CLOCK_REALTIME, &iDetail);
	localtime_r(&iDetail.tv_sec, &iInfo);

	nYear		= iInfo.tm_year + 1900;
	nMonth		= iInfo.tm_mon + 1;
	nDay	 	= iInfo.tm_mday;
	nWeek		= iInfo.tm_wday;
	nHour		= iInfo.tm_hour;
	nMin		= iInfo.tm_min;
	nSec		= iInfo.tm_sec;
	nMSec		= iDetail.tv_nsec / 1000000;
#endif
}

DateTime::DateTime(const DateTime & r)
	: nYear(r.nYear)
	, nMonth(r.nMonth)
	, nDay(r.nDay)
	, nWeek(r.nWeek)
	, nHour(r.nHour)
	, nMin(r.nMin)
	, nSec(r.nSec)
	, nMSec(r.nMSec) {}

DateTime::DateTime(int nY, int nM, int nD, int nH, int nMin, int nS)
	: nYear(nY)
	, nMonth(nM)
	, nDay(nD)
	, nWeek(0)
	, nHour(nH)
	, nMin(nMin)
	, nSec(nS)
	, nMSec(0) {
	struct tm	iTime;

	iTime.tm_year	= nY - 1900;
	iTime.tm_mon	= nM - 1;
	iTime.tm_mday	= nD;
	iTime.tm_hour	= nH;
	iTime.tm_min	= nMin;
	iTime.tm_sec	= nS;

	mktime(&iTime);
	nWeek = iTime.tm_wday;
}

DateTime::DateTime(uint64_t nHRTime) {
	time_t nTime = (time_t)(nHRTime / 1000);
	struct tm iTime;

#if defined(_WIN32)
	localtime_s(&iTime, &nTime);
#else
	localtime_r(&nTime, &iTime);
#endif

	nYear		= iTime.tm_year + 1900;
	nMonth		= iTime.tm_mon + 1;
	nDay		= iTime.tm_mday;
	nWeek		= iTime.tm_wday;
	nHour		= iTime.tm_hour;
	nMin		= iTime.tm_min;
	nSec		= iTime.tm_sec;
	nMSec		= nTime % 1000;
}

uint64_t DateTime::Time() {
	struct tm	iTime;

	iTime.tm_year	= nYear - 1900;
	iTime.tm_mon	= nMonth - 1;
	iTime.tm_mday	= nDay;
	iTime.tm_hour	= nHour;
	iTime.tm_min	= nMin;
	iTime.tm_sec	= nSec;

	return mktime(&iTime) * 1000 + nMSec;
}


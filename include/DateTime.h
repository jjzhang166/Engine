#ifndef		__ENGINE_DATETIME_H_INCLUDED__
#define		__ENGINE_DATETIME_H_INCLUDED__

#include	<cstdint>
#include	<string>

struct DateTime {
	int	nYear;
	int	nMonth;
	int	nDay;
	int nWeek;
	int	nHour;
	int	nMin;
	int	nSec;
	int	nMSec;

	DateTime();
	DateTime(uint64_t nHRTime);
	DateTime(const DateTime & r);
	DateTime(int nY, int nM, int nD, int nH, int nMin, int nS);

	/**
	 * Return a high resolution time for serialize.
	 **/
	uint64_t Time();
};

/**
 * Get high resolution CPU time(in milliseconds).
 **/
double Tick();

#endif//!	__ENGINE_DATETIME_H_INCLUDED__
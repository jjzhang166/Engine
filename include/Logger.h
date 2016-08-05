#ifndef		__ENGINE_LOGGER_H_INCLUDED__
#define		__ENGINE_LOGGER_H_INCLUDED__

#define		LOG_DEBUG(pFmt, ...)	Logger::Instance().Log(ELog::Debug, __FILE__, __LINE__, pFmt, ##__VA_ARGS__)
#define		LOG_INFO(pFmt, ...)		Logger::Instance().Log(ELog::Info, __FILE__, __LINE__, pFmt, ##__VA_ARGS__)
#define		LOG_WARN(pFmt, ...)		Logger::Instance().Log(ELog::Warning, __FILE__, __LINE__, pFmt, ##__VA_ARGS__)
#define		LOG_ERR(pFmt, ...)		Logger::Instance().Log(ELog::Error, __FILE__, __LINE__, pFmt, ##__VA_ARGS__)

#include	<string>
#include	<mutex>

namespace ELog {
	enum Level {
		Error = 0,
		Warning,
		Info,
		Debug
	};
}

class Logger {
public:
	Logger();
	virtual ~Logger();

	/**
	 * Get runtime singleton instance of Logger.
	 **/
	static Logger & Instance();

	/**
	 * Initailize logger environment.
	 *
	 * \param	sName		Name of logger file. {sPath}/{Date}/{sName}_{Time}.log
	 * \param	sPath		Root folder contains this logger file.
	 * \param	nMaxSize	Max size of a single logger file. If current logger file is larger than that a new file will be generated.
	 * \param	emMaxLevel	Data filter.
	 * \param	bStdOut		Should we use std::cout to show this logger as well.
	 **/	
	void	Init(
				const std::string & sName = "main", 
				const std::string & sPath = "logs",
				size_t nMaxSize = 1024 * 1024 * 4, 
				ELog::Level emMaxLevel = ELog::Info, 
				bool bStdOut = true);

	/**
	 * Record a single log. Use LOG_XXX instead.
	 *
	 * \param	emLevel		Filter of this log message.
	 * \param	pFile		__FILE__
	 * \param	nLine		__LINE__
	 * \param	pFmt		Message or format for this message.
	 * \param	...			Addition parameters.
	 **/ 
	void	Log(
				ELog::Level emLevel,
				const char * pFile,
				int nLine,
				const char * pFmt,
				...);

private:
	void	__Create(struct DateTime * pCreate);

private:
	FILE *			_pFile;
	char *			_pBuf;
	std::string		_sName;
	std::string		_sPath;
	size_t			_nMaxSize;
	ELog::Level		_emMaxLevel;
	bool			_bStdOut;
	int				_nLastDay;
	size_t			_nWrited;
	std::mutex		_iLock;
};

#endif//!	__ENGINE_LOGGER_H_INCLUDED__
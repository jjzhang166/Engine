#ifndef		__ENGINE_REDIS_H_INCLUDED__
#define		__ENGINE_REDIS_H_INCLUDED__

#include	<cstdint>
#include	<functional>
#include	<map>
#include	<string>
#include	<vector>

#define		GRedis	Redis::Instance()

class Redis {
public:
	typedef std::function<void(int64_t, bool)>				CBInt;	//! Integer callback. params are "retval" & "does_exists"
	typedef std::function<void(const char *, size_t)>		CBStr;	//! String callback. params are "data_pointer" & "size_of_data"
	typedef std::function<void(std::vector<std::string> &)>	CBArr;	//! Array callback. params are "array"

public:
	Redis();
	virtual ~Redis();

	static Redis &	Instance();

	bool	Connect(const std::string & sHost, int nPort);
	bool	IsConnected();
	void	Close();
	void	Breath(); //! NOTE: If using GRedis in application based on Application. You should never call this.

	bool	Command(const char * pFmt, ...);
	bool	Command(CBInt fOpt, const char * pFmt, ...);
	bool	Command(CBStr fOpt, const char * pFmt, ...);
	bool	Command(CBArr fOpt, const char * pFmt, ...);

	bool	Get(const std::string & sKey, CBStr fOpt) { return this->Command(fOpt, "GET %s", sKey.c_str()); }
	bool	Set(const std::string & sKey, const std::string & sData) { return this->Set(sKey, sData.data(), sData.size()); }
	bool	Set(const std::string & sKey, const char * pBuf, size_t nSize) { return this->Command("SET %s %b", sKey.c_str(), pBuf, nSize); }
	bool	Del(const std::string & sKey) { return this->Command("DEL %s", sKey.c_str()); }
			
private:
	struct redisAsyncContext *	_pCtx;
	struct ev_loop *			_pLoop;
	int							_emState;
	uint64_t					_nAllocId;

	std::map<uint64_t, CBInt>	_mCBInt;
	std::map<uint64_t, CBStr>	_mCBStr;
	std::map<uint64_t, CBArr>	_mCBArr;

	std::string		_sHost;
	int				_nPort;
};

#endif//!	__ENGINE_REDIS_H_INCLUDED__

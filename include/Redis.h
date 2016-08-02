#ifndef		__ENGINE_REDIS_H_INCLUDED__
#define		__ENGINE_REDIS_H_INCLUDED__

#include	<cstdint>
#include	<functional>
#include	<map>
#include	<string>

#define		GRedis	Redis::Instance()

namespace ERedis {
	enum State {
		Success	= 0,
		Running,
		ConnectError,
		AllocError,
		EventError
	};
}

class Redis {
public:
	Redis();
	virtual ~Redis();

	static Redis &	Instance();

	int		Connect(const std::string & sHost, int nPort);
	bool	IsConnected() { return _bConnected; }
	void	Close();
	void	Breath(); //! NOTE: If using GRedis in application based on Application. You should never call this.

	bool	Get(const std::string & sKey, std::function<void (const char *, size_t)> fOpt);
	bool	Set(const std::string & sKey, const char * pBuf, size_t nSize);
	bool	Del(const std::string & sKey);

private:
	void	__Reconnect();
		
private:
	struct redisAsyncContext * _pCtx;
	struct ev_loop * _pLoop;
	bool _bRunning;
	bool _bConnected;
	uint64_t _nAllocId;

	std::map<uint64_t, std::function<void(const char *, size_t)>> _mGetter;
};

#endif//!	__ENGINE_REDIS_H_INCLUDED__

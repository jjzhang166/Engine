#include	<Redis.h>
#include	<Logger.h>
#include	<memory>

#include	"hiredis/net.h"
#include	"hiredis/hiredis_ev.h"

Redis::Redis() : _pCtx(nullptr), _pLoop(ev_loop_new(EVFLAG_AUTO)), _bRunning(false), _bConnected(false), _nAllocId(1) {
#if defined(_WIN32)
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) throw std::runtime_error("Create REDIS client instance error WSAStartup!");
#endif
}

Redis::~Redis() {
	Close();
	if (_pLoop) ev_loop_destroy(_pLoop);

#if defined(_WIN32)
	WSACleanup();
#endif
}

Redis & Redis::Instance() {
	static std::unique_ptr<Redis> _iIns;
	if (!_iIns.get()) _iIns.reset(new Redis);
	return *(_iIns.get());
}

int Redis::Connect(const std::string & sHost, int nPort) {
	if (_bRunning) return ERedis::Running;
	
	_pCtx = redisAsyncConnect(sHost.c_str(), nPort);
	if (!_pCtx) {
		return ERedis::AllocError;
	} else if (_pCtx->err) {
		redisAsyncFree(_pCtx);
		return ERedis::ConnectError;
	} else {
		_pCtx->data = this;
	}

	/// Attach to RedisAE
	redisLibevAttach(_pLoop, _pCtx);

	/// Set connect result handler.
	redisAsyncSetConnectCallback(_pCtx, [](const redisAsyncContext * pCtx, int nStatus) {
		Redis * p = static_cast<Redis *>(pCtx->data);
		if (nStatus != REDIS_OK) {
			LOG_WARN("Connect to redis server failed! Reason : %s", pCtx->errstr);
			p->__Reconnect();
		} else {
			LOG_INFO("Successfully connect to redis server[%s:%d]!", pCtx->c.tcp.host, pCtx->c.tcp.port);
			p->_bConnected = true;
		}
	});

	/// Hook disconnect event.
	redisAsyncSetDisconnectCallback(_pCtx, [](const redisAsyncContext * pCtx, int nStatus) {
		Redis * p = static_cast<Redis *>(pCtx->data);
		p->_bConnected = false;
		if (nStatus != REDIS_OK) LOG_ERR("Disconnect to redis server due to error : %s", pCtx->errstr);
		p->__Reconnect();
	});
		
	/// Waiting for connect result.
	ev_run(_pLoop, EVRUN_ONCE);
	ev_run(_pLoop, EVRUN_NOWAIT);
	
	_bRunning = true;
	return ERedis::Success;
}

void Redis::Close() {
	if (!_bRunning) return;

	_bRunning = false;

	redisAsyncDisconnect(_pCtx);
	ev_run(_pLoop, EVRUN_ONCE);
	ev_run(_pLoop, EVRUN_NOWAIT);
	redisAsyncFree(_pCtx);
}

void Redis::Breath() {
	if (!_bRunning) return;
	ev_run(_pLoop, EVRUN_NOWAIT);
}

bool Redis::Get(const std::string & sKey, std::function<void(const char *, size_t)> fOpt) {
	if (!_bConnected) {
		LOG_ERR("Try to GET data on a redis NOT connected!");
		return false;
	}

	uint64_t nId = _nAllocId++;
	if (fOpt) _mGetter.insert(std::pair<uint64_t, std::function<void(const char *, size_t)>>(nId, fOpt));

	int nState = redisAsyncCommand(_pCtx, [](redisAsyncContext * pCtx, void * pData, void * pUser) {
		Redis * p = static_cast<Redis *>(pCtx->data);
		redisReply * pReply = (redisReply *)pData;
		uint64_t n = (uint64_t)(pUser);
		auto it = p->_mGetter.find(n);

		if (!pReply) {
			LOG_ERR("Redis GET error with NO reply!!!");
		} else if (pReply->type == REDIS_REPLY_ERROR) {
			LOG_ERR("Redis GET error : %s", pReply->str);
		} else if (pReply->type == REDIS_REPLY_NIL) {
			if (it != p->_mGetter.end()) it->second(NULL, 0);
		} else if (pReply->type != REDIS_REPLY_STRING) {
			LOG_ERR("Redis GET error, string expacted but got : %d", pReply->type);
		} else {
			if (it != p->_mGetter.end()) it->second(pReply->str, pReply->len);
		}

		p->_mGetter.erase(n);
	}, (void *)(uintptr_t)nId, "GET %s", sKey.c_str());

	if (nState) {
		LOG_ERR("Redis GET error with code : %d", nState);
		return false;
	}

	return true;
}

bool Redis::Set(const std::string & sKey, const char * pBuf, size_t nSize) {
	if (!_bConnected) {
		LOG_ERR("Try to SET data on a redis NOT connected!");
		return false;
	}

	int nState = redisAsyncCommand(_pCtx, [](redisAsyncContext * pCtx, void * pData, void * pUser) {
		redisReply * pReply = (redisReply *)pData;

		if (!pReply) {
			LOG_ERR("Redis SET error with NO reply!!!");
		} else if (pReply->type == REDIS_REPLY_ERROR) {
			LOG_ERR("Redis SET error : %s", pReply->str);
		} else if (pReply->type != REDIS_REPLY_STATUS) {
			LOG_ERR("Redis SET error with wrong status : %d != REDIS_REPLY_STATUS(%d)", pReply->type, REDIS_REPLY_STATUS);
		}
	}, NULL, "SET %s %b", sKey.c_str(), pBuf, nSize);

	if (nState) {
		LOG_ERR("Redis SET error with code : %d", nState);
		return false;
	}

	return true;
}

bool Redis::Del(const std::string & sKey) {
	if (!_bConnected) {
		LOG_ERR("Try to DEL data on a redis NOT connected!");
		return false;
	}

	int nState = redisAsyncCommand(_pCtx, [](redisAsyncContext * pCtx, void * pData, void * pUser) {
		redisReply * pReply = (redisReply *)pData;

		if (!pReply) {
			LOG_ERR("Redis DEL error with NO reply!!!");
		} else if (pReply->type == REDIS_REPLY_ERROR) {
			LOG_ERR("Redis DEL error : %s", pReply->str);
		} else if (pReply->type != REDIS_REPLY_STATUS) {
			LOG_ERR("Redis DEL error with wrong status : %d != REDIS_REPLY_STATUS(%d)", pReply->type, REDIS_REPLY_STATUS);
		}
	}, NULL, "DEL %s", sKey.c_str());

	if (nState) {
		LOG_ERR("Redis DEL error with code : %d", nState);
		return false;
	}

	return true;
}

void Redis::__Reconnect() {
	if (!_bRunning || _bConnected) return;

	/// Directly call connect without initialization (Because we have initialized it before).
	redisContextConnectTcp(&_pCtx->c, _pCtx->c.tcp.host, _pCtx->c.tcp.port, NULL);

	/// Copy errors.
	redisContext *c = &(_pCtx->c);
	_pCtx->err = c->err;
	_pCtx->errstr = c->errstr;
}

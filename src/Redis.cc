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

bool Redis::Command(const char * pFmt, ...) {
	if (!_bConnected) {
		LOG_ERR("Try to run command on a redis NOT connected!");
		return false;
	}

	va_list args;
	va_start(args, pFmt);

	int nState = redisvAsyncCommand(_pCtx, [](redisAsyncContext * pCtx, void * pData, void * pUser) {
		redisReply * pReply = (redisReply *)pData;

		if (!pReply) {
			LOG_ERR("Redis run command error with NO reply!!!");
		} else if (pReply->type == REDIS_REPLY_ERROR) {
			LOG_ERR("Redis run command error : %s", pReply->str);
		} else if (pReply->type != REDIS_REPLY_STATUS) {
			LOG_ERR("Redis run command error with wrong status : %d != REDIS_REPLY_STATUS(%d)", pReply->type, REDIS_REPLY_STATUS);
		}
	}, NULL, pFmt, args);

	va_end(args);

	if (nState) {
		LOG_ERR("Redis run command error with code : %d", nState);
		return false;
	}

	return true;
}

bool Redis::Command(Redis::CBInt fOpt, const char * pFmt, ...) {
	if (!_bConnected) {
		LOG_ERR("Try to run command on a redis NOT connected!");
		return false;
	}

	va_list args;
	va_start(args, pFmt);

	uint64_t nId = _nAllocId++;
	if (fOpt) _mCBInt.insert(std::pair<uint64_t, CBInt>(nId, fOpt));

	int nState = redisvAsyncCommand(_pCtx, [](redisAsyncContext * pCtx, void * pData, void * pUser) {
		Redis * p = static_cast<Redis *>(pCtx->data);
		redisReply * pReply = (redisReply *)pData;
		uint64_t n = (uint64_t)(pUser);
		auto it = p->_mCBInt.find(n);

		if (!pReply) {
			LOG_ERR("Redis run command error with NO reply!!!");
		} else if (pReply->type == REDIS_REPLY_ERROR) {
			LOG_ERR("Redis run command error : %s", pReply->str);
		} else if (pReply->type == REDIS_REPLY_NIL) {
			if (it != p->_mCBInt.end()) it->second(0, false);
		} else if (pReply->type != REDIS_REPLY_INTEGER) {
			LOG_ERR("Redis run command error, integer expacted but got : %d", pReply->type);
		} else {
			if (it != p->_mCBInt.end()) it->second(pReply->integer, true);
		}

		p->_mCBInt.erase(n);
	}, (void *)(uintptr_t)nId, pFmt, args);

	va_end(args);

	if (nState) {
		LOG_ERR("Redis run command error with code : %d", nState);
		return false;
	}

	return true;
}

bool Redis::Command(Redis::CBStr fOpt, const char * pFmt, ...) {
	if (!_bConnected) {
		LOG_ERR("Try to run command on a redis NOT connected!");
		return false;
	}

	va_list args;
	va_start(args, pFmt);

	uint64_t nId = _nAllocId++;
	if (fOpt) _mCBStr.insert(std::pair<uint64_t, CBStr>(nId, fOpt));

	int nState = redisvAsyncCommand(_pCtx, [](redisAsyncContext * pCtx, void * pData, void * pUser) {
		Redis * p = static_cast<Redis *>(pCtx->data);
		redisReply * pReply = (redisReply *)pData;
		uint64_t n = (uint64_t)(pUser);
		auto it = p->_mCBStr.find(n);

		if (!pReply) {
			LOG_ERR("Redis run command error with NO reply!!!");
		} else if (pReply->type == REDIS_REPLY_ERROR) {
			LOG_ERR("Redis run command error : %s", pReply->str);
		} else if (pReply->type == REDIS_REPLY_NIL) {
			if (it != p->_mCBStr.end()) it->second(nullptr, 0);
		} else if (pReply->type != REDIS_REPLY_STRING) {
			LOG_ERR("Redis run command error, string expacted but got : %d", pReply->type);
		} else {
			if (it != p->_mCBStr.end()) it->second(pReply->str, pReply->len);
		}

		p->_mCBStr.erase(n);
	}, (void *)(uintptr_t)nId, pFmt, args);

	va_end(args);

	if (nState) {
		LOG_ERR("Redis run command error with code : %d", nState);
		return false;
	}

	return true;
}

bool Redis::Command(Redis::CBArr fOpt, const char * pFmt, ...) {
	if (!_bConnected) {
		LOG_ERR("Try to run command on a redis NOT connected!");
		return false;
	}

	va_list args;
	va_start(args, pFmt);

	uint64_t nId = _nAllocId++;
	if (fOpt) _mCBArr.insert(std::pair<uint64_t, CBArr>(nId, fOpt));

	int nState = redisvAsyncCommand(_pCtx, [](redisAsyncContext * pCtx, void * pData, void * pUser) {
		Redis * p = static_cast<Redis *>(pCtx->data);
		redisReply * pReply = (redisReply *)pData;
		uint64_t n = (uint64_t)(pUser);
		auto it = p->_mCBArr.find(n);
		std::vector<std::string> vRet;

		if (!pReply) {
			LOG_ERR("Redis run command error with NO reply!!!");
		} else if (pReply->type == REDIS_REPLY_ERROR) {
			LOG_ERR("Redis run command error : %s", pReply->str);
		} else if (pReply->type == REDIS_REPLY_NIL) {
			if (it != p->_mCBArr.end()) it->second(vRet);
		} else if (pReply->type != REDIS_REPLY_ARRAY) {
			LOG_ERR("Redis run command error, array expacted but got : %d", pReply->type);
		} else {
			for (size_t i = 0; i < pReply->elements; ++i) {
				redisReply * pChild = pReply->element[i];
				if (pChild->type == REDIS_REPLY_NIL) {
					vRet.push_back("");
				} else if (pChild->type != REDIS_REPLY_STRING) {
					LOG_ERR("Redis run command error, element must be string value but got : %d", pChild->type);
				} else {
					vRet.push_back(std::string(pChild->str, pChild->len));
				}
			}

			if (it != p->_mCBArr.end()) it->second(vRet);
		}

		p->_mCBArr.erase(n);
	}, (void *)(uintptr_t)nId, pFmt, args);

	va_end(args);

	if (nState) {
		LOG_ERR("Redis run command error with code : %d", nState);
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

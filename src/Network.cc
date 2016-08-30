#include	<Network.h>
#include	<Guard.h>
#include	<Logger.h>

#include	<atomic>
#include	<cstdlib>
#include	<cstring>
#include	<map>

#if defined(_WIN32)
#	if !defined(MSG_DONTWAIT)
#		define		MSG_DONTWAIT	0
#	endif
#	define		SOCKET_ERR		WSAGetLastError()
#	define		FD_SETSIZE		4096
#	include		<WinSock2.h>
#	include		<WS2tcpip.h>
#	pragma		comment(lib, "ws2_32.lib")

typedef int socklen_t;

#else
#	define		INVALID_SOCKET	-1
#	define		SOCKET_ERR		errno
#	define		closesocket(s)	close(s)
#	include		<arpa/inet.h>
#	include		<fcntl.h>
#	include		<netinet/in.h>
#	include		<sys/epoll.h>
#	include		<sys/socket.h>
#	include		<sys/types.h>
#	include		<unistd.h>
#endif

using namespace std;

struct SocketData {
	uint64_t	nConnId;
	int32_t		nSize;
	uint8_t		nUsed;
};

struct ConnectionInfo {
#if defined(_WIN32)
	SOCKET		nSocket;
#else
	int			nSocket;
#endif
	sockaddr_in	iAddr;
};

typedef map<uint64_t, ConnectionInfo> Connections;

class SocketBuffer {
public:
	SocketBuffer(size_t nCapacity);
	virtual ~SocketBuffer();

	bool Write(void * pData, size_t nSize);
	bool Read(char ** pData, size_t & nSize);

private:
	atomic<int>	_nCur;
	size_t		_nCapacity;
	char *		_pMem[2];
};

SocketBuffer::SocketBuffer(size_t nCapacity)
	: _nCur(0)
	, _nCapacity(nCapacity) {
	_pMem[0] = new char[nCapacity];
	_pMem[1] = new char[nCapacity];

	memset(_pMem[0], 0, nCapacity);
	memset(_pMem[1], 0, nCapacity);
}

SocketBuffer::~SocketBuffer() {
	delete[] _pMem[0];
	delete[] _pMem[1];
}

bool SocketBuffer::Write(void * pData, size_t nSize) {
	char * pMem = _pMem[_nCur.load()];
	size_t * pSize = (size_t *)pMem;
	size_t nWrited = *pSize;
	if (nWrited + nSize > _nCapacity) return false;
	*pSize = nWrited + nSize;
	memcpy(pMem + sizeof(size_t) + nWrited, pData, nSize);
	return true;
}

bool SocketBuffer::Read(char ** pData, size_t & nSize) {
	int nPrev = _nCur.load();
	memset(_pMem[1 - nPrev], 0, _nCapacity);
	_nCur.exchange(1 - nPrev);
	nSize = *((size_t *)(_pMem[nPrev]));
	*pData = _pMem[nPrev] + sizeof(size_t);
	return nSize > 0;
}

class SocketGuard {
public:
	SocketGuard(ISocket * p, const string & sHost, int nPort);
	virtual ~SocketGuard();

	void	Start();

private:
	ISocket *	_p;
	string	_sHost;
	int			_nPort;
	thread *	_pWorker;
	bool		_bRunning;
};

SocketGuard::SocketGuard(ISocket * p, const string & sHost, int nPort)
	: _p(p)
	, _sHost(sHost)
	, _nPort(nPort)
	, _pWorker(nullptr)
	, _bRunning(false) {}

SocketGuard::~SocketGuard() {
	_bRunning = false;
	if (_pWorker) {
		if (_pWorker->joinable()) _pWorker->join();
		delete _pWorker;
	}
}

void SocketGuard::Start() {
	if (_bRunning) return;
	_bRunning = true;

	if (_pWorker) {
		if (_pWorker->joinable()) _pWorker->join();
		delete _pWorker;
	}

	_pWorker = new thread([this]() {
		while (_bRunning) {
			if (!_p->IsConnected()) {
				int n = _p->Connect(_sHost, _nPort, false);
				if (n != ENet::Success) LOG_WARN("Try to reconnect to [%s:%d] ... %d", _sHost.c_str(), _nPort, n);
			}

			this_thread::sleep_for(chrono::seconds(1));
		}
	});
}

class SocketContext {
public:
	SocketContext(ISocket * pOwner);
	virtual ~SocketContext();

	int		Connect(const string & sIP, int nPort);
	bool	IsConnected() { return _nSocket != INVALID_SOCKET; }
	void	Close(ENet::Close emCode);
	bool	Send(const char * pData, size_t nSize);
	void	Breath();

private:
	void	__IOThread();

private:
	ISocket *		_pOwner;
	SocketBuffer	_iBuffer;
	thread *		_pIOWorker;

#if defined(_WIN32)	
	SOCKET			_nSocket;
#else
	int				_nSocket;
#endif
};

class ServerSocketContext {
public:
	ServerSocketContext(IServerSocket * pOwner);
	virtual ~ServerSocketContext();

	int		Listen(const string & sIP, int nPort);
	bool	Send(uint64_t nConnId, const char * pData, size_t nSize);
	void	Broadcast(const char * pData, size_t nSize);
	void	Close(uint64_t nConnId, ENet::Close emCode);
	void	Shutdown();
	void	Breath();

	IServerSocket::RemoteInfo	GetClientInfo(uint64_t nConnId);

private:
	void	__AcceptThread();
	void	__IOThread();

private:
	IServerSocket *	_pOwner;
	SocketBuffer	_iBuffer;
	thread *		_pAcceptWorker;
	thread *		_pIOWorker;
	Connections		_mConns;

#if defined(_WIN32)
	SOCKET			_nSocket;
	fd_set			_tIO;
#else
	int				_nSocket;
	int				_tIO;
#endif
};

SocketContext::SocketContext(ISocket * pOwner)
	: _pOwner(pOwner)
	, _iBuffer(4 * 1024 * 1024)
	, _pIOWorker(nullptr)
	, _nSocket(INVALID_SOCKET) {
#if defined(_WIN32)
	WORD wVer = MAKEWORD(2, 2);
	WSADATA wData;
	if (WSAStartup(wVer, &wData)) throw runtime_error("WSAStartup failed!");
#endif
}

SocketContext::~SocketContext() {
	Close(ENet::Local);
#if defined(_WIN32)
	WSACleanup();
#endif
}

int SocketContext::Connect(const string & sIP, int nPort) {
	if (_nSocket != INVALID_SOCKET) return ENet::Running;
	if ((_nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) return SOCKET_ERR;

	Guard iClean([this](){ closesocket(_nSocket); _nSocket = INVALID_SOCKET; });

#if defined(_WIN32)
	u_long nFlags = 1;
	if (ioctlsocket(_nSocket, FIONBIO, &nFlags) != 0) return SOCKET_ERR;
#else
	int nFlags = fcntl(_nSocket, F_GETFL);
	fcntl(_nSocket, F_SETFL, O_NONBLOCK | nFlags);
#endif

	struct sockaddr_in iAddr;
	memset(&iAddr, 0, sizeof(iAddr));

	iAddr.sin_family	= AF_INET;
	iAddr.sin_port		= htons(nPort);

#if defined(_WIN32) && !defined(_MSC_VER)
	iAddr.sin_addr.s_addr = inet_addr(sIP.c_str());
#else
	if (0 >= inet_pton(AF_INET, sIP.c_str(), &iAddr.sin_addr)) return ENet::BadParam;
#endif
	
	if (connect(_nSocket, (struct sockaddr *)&iAddr, sizeof(struct sockaddr)) < 0) {
		int nErr = SOCKET_ERR;
	#if defined(_WIN32)
		if (nErr != WSAEWOULDBLOCK) return nErr;
	#else
		if (nErr != EINPROGRESS) return nErr;
	#endif

		fd_set iSet;
		struct timeval iWait;

		iWait.tv_sec	= 4;
		iWait.tv_usec	= 0;

		FD_ZERO(&iSet);
		FD_SET(_nSocket, &iSet);

		if (select(0, 0, &iSet, 0, &iWait) <= 0) return ENet::Timeout;

	#if !defined(_WIN32)
		socklen_t nLen;
		if (getsockopt(_nSocket, SOL_SOCKET, SO_ERROR, &nErr, (socklen_t *) &nLen) < 0 || nErr != 0)
			return ENet::Closed;
	#endif
	}

	try {
		_pIOWorker = new thread(&SocketContext::__IOThread, this);
	} catch (runtime_error &) {
		return ENet::Thread;
	}

	iClean.Dismiss();
	_pOwner->OnConnected();
	return 0;
}

void SocketContext::Close(ENet::Close emCode) {
	if (_nSocket == INVALID_SOCKET) return;
	closesocket(_nSocket);
	_nSocket = INVALID_SOCKET;

	if (this_thread::get_id() != _pIOWorker->get_id()) {
		if (_pIOWorker->joinable()) _pIOWorker->join();
		delete _pIOWorker;
	}

	_pOwner->OnClose(emCode);
}

bool SocketContext::Send(const char * pData, size_t nSize) {
	if (_nSocket == INVALID_SOCKET) return false;

	char *	pSend = (char *)pData;
	int		nSend = 0;
	size_t	nLeft = nSize;

	while (true) {
		nSend = send(_nSocket, pSend, (int)nLeft, MSG_DONTWAIT);
		if (nSend < 0) {
			int nErr = SOCKET_ERR;
		#if defined(_WIN32)
			if (nErr == WSAEWOULDBLOCK) {
		#else
			if (nErr == EAGAIN) {
		#endif
				this_thread::sleep_for(chrono::nanoseconds(1));
			} else {
				return false;
			}
		} else if ((size_t) nSend < nLeft) {
			nLeft -= nSend;
			pSend += nSend;
		} else {
			break;
		}
	}

	return nLeft == 0;
}

void SocketContext::Breath() {
	if (_nSocket == INVALID_SOCKET) return;

	size_t nSize = 0;
	char * pData = nullptr;

	if (!_iBuffer.Read(&pData, nSize)) return;
	_pOwner->OnReceive(pData, nSize);
}

void SocketContext::__IOThread() {
	char * pBuf	= new char[65536];
	int nCurRead = 0;

	while (_nSocket != INVALID_SOCKET) {
		memset(pBuf, 0, 65536);

		nCurRead = recv(_nSocket, pBuf, 65536, MSG_DONTWAIT);
		if (nCurRead > 0) {
			while (!_iBuffer.Write(pBuf, nCurRead)) this_thread::sleep_for(chrono::nanoseconds(1));
	#if defined(_WIN32)
		} else if (nCurRead < 0 && WSAEWOULDBLOCK == WSAGetLastError()) {
	#else
		} else if (nCurRead < 0 && errno == EAGAIN) {
	#endif
			this_thread::sleep_for(chrono::nanoseconds(1));
		} else {
			Close(nCurRead == 0 ? ENet::Remote : ENet::BadData);
			break;
		}
	}

	delete[] pBuf;
}

ServerSocketContext::ServerSocketContext(IServerSocket * pOwner)
	: _pOwner(pOwner)
	, _iBuffer(8 * 1024 * 1024)
	, _pAcceptWorker(nullptr)
	, _pIOWorker(nullptr)
	, _mConns()
	, _nSocket(INVALID_SOCKET)
#if defined(_WIN32)
	, _tIO()
#else
	, _tIO(-1)
#endif
	{
#if defined(_WIN32)
	WORD wVer = MAKEWORD(2, 2);
	WSADATA wData;
	if (WSAStartup(wVer, &wData)) throw runtime_error("WSAStartup failed!");
#endif
}

ServerSocketContext::~ServerSocketContext() {
	Shutdown();
#if defined(_WIN32)
	WSACleanup();
#endif
}

int ServerSocketContext::Listen(const string & sIP, int nPort) {
	if (_nSocket != INVALID_SOCKET) return ENet::Running;
	if ((_nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) return SOCKET_ERR;

#if defined(_WIN32)
	Guard iClean([this]() { closesocket(_nSocket); _nSocket = INVALID_SOCKET; });

	u_long nFlags = 1;
	if (ioctlsocket(_nSocket, FIONBIO, &nFlags) != 0) return SOCKET_ERR;

	FD_ZERO(&_tIO);
#else
	if ((_tIO = epoll_create(1)) == -1) {
		close(_nSocket); _nSocket = INVALID_SOCKET;
		return errno;
	}

	int nFlags = fcntl(_nSocket, F_GETFL);
	fcntl(_nSocket, F_SETFL, O_NONBLOCK | nFlags);

	Guard iClean([this]() { close(_tIO); close(_nSocket); _nSocket = INVALID_SOCKET; });
#endif

	struct sockaddr_in iAddr;
	memset(&iAddr, 0, sizeof(iAddr));

	iAddr.sin_family		= AF_INET;
	iAddr.sin_port			= htons(nPort);

#if defined(_WIN32) && !defined(_MSC_VER)
	iAddr.sin_addr.s_addr = inet_addr(sIP.c_str());
#else
	if (0 >= inet_pton(AF_INET, sIP.c_str(), &iAddr.sin_addr)) return ENet::BadParam;
#endif

	if (::bind(_nSocket, (sockaddr *) &iAddr, sizeof(sockaddr)) < 0) return SOCKET_ERR;
	if (::listen(_nSocket, 512) < 0) return SOCKET_ERR;

	try {
		_pAcceptWorker = new thread(&ServerSocketContext::__AcceptThread, this);
		_pIOWorker = new thread(&ServerSocketContext::__IOThread, this);
	} catch (runtime_error &) {
		if (_pAcceptWorker) {			
			iClean.Dismiss();
		#if !defined(_WIN32)
			close(_tIO);
		#endif
			closesocket(_nSocket);
			_nSocket = INVALID_SOCKET;

			if (_pAcceptWorker->joinable()) _pAcceptWorker->join();
			delete _pAcceptWorker;
		}

		return ENet::Thread;
	}
	
	iClean.Dismiss();
	return 0;
}

bool ServerSocketContext::Send(uint64_t nConnId, const char * pData, size_t nSize) {
	auto it = _mConns.find(nConnId);
	if (it == _mConns.end()) return false;

	auto	nSocket	= it->second.nSocket;
	char *	pSend	= (char *)pData;
	int		nSend	= 0;
	size_t	nLeft	= nSize;

	while (true) {
		nSend = send(nSocket, pSend, (int)nLeft, MSG_DONTWAIT);
		if (nSend < 0) {
			int nErr = SOCKET_ERR;
		#if defined(_WIN32)
			if (nErr == WSAEWOULDBLOCK) {
		#else
			if (nErr == EAGAIN) {
		#endif
				this_thread::sleep_for(chrono::nanoseconds(1));
			} else {
				return false;
			}
		} else if ((size_t) nSend < nLeft) {
			nLeft -= nSend;
			pSend += nSend;
		} else {
			break;
		}
	}

	return nLeft == 0;
}

void ServerSocketContext::Broadcast(const char * pData, size_t nSize) {
	for (auto it = _mConns.begin(); it != _mConns.end(); ++it) {
		auto	nSocket	= it->second.nSocket;
		char *	pSend	= (char *)pData;
		int		nSend	= 0;
		size_t	nLeft	= nSize;

		while (true) {
			nSend = send(nSocket, pSend, (int)nLeft, MSG_DONTWAIT);
			if (nSend < 0) {
				int nErr = SOCKET_ERR;
			#if defined(_WIN32)
				if (nErr == WSAEWOULDBLOCK) {
			#else
				if (nErr == EAGAIN) {
			#endif
					this_thread::sleep_for(chrono::nanoseconds(1));
				} else {
					break;
				}
			} else if ((size_t) nSend < nLeft) {
				nLeft -= nSend;
				pSend += nSend;
			} else {
				break;
			}
		}
	}
}

void ServerSocketContext::Close(uint64_t nConnId, ENet::Close emCode) {
	auto it = _mConns.find(nConnId);
	if (it == _mConns.end()) return;

	_pOwner->OnClose(nConnId, emCode);

#if defined(_WIN32)
	FD_CLR(it->second.nSocket, &_tIO);
#else
	epoll_ctl(_tIO, EPOLL_CTL_DEL, it->second.nSocket, NULL);
#endif

	closesocket(it->second.nSocket);
	_mConns.erase(it);
}

void ServerSocketContext::Shutdown() {
	if (_nSocket == INVALID_SOCKET) return;

	for (auto it = _mConns.begin(); it != _mConns.end(); ++it) Close(it->first, ENet::Local);
	_mConns.clear();

#if defined(_WIN32)
	FD_ZERO(&_tIO);
#else
	close(_tIO);
#endif

	closesocket(_nSocket);
	_nSocket = INVALID_SOCKET;

	if (_pAcceptWorker->joinable()) _pAcceptWorker->join();
	if (_pIOWorker->joinable()) _pIOWorker->join();
	delete _pAcceptWorker;
	delete _pIOWorker;
}

void ServerSocketContext::Breath() {
	if (_nSocket == INVALID_SOCKET) return;

	size_t nOffset = 0;
	char * pData = nullptr;

	if (!_iBuffer.Read(&pData, nOffset)) return;

	for (size_t nReaded = 0; nReaded < nOffset;) {
		SocketData * pHeader = (SocketData *)(pData + nReaded);
		while (pHeader->nUsed == 0) this_thread::sleep_for(chrono::nanoseconds(1));

		char * pCont = pData + nReaded + sizeof(SocketData);
		_pOwner->OnReceive(pHeader->nConnId, pCont, pHeader->nSize - sizeof(SocketData));
		nReaded += pHeader->nSize;
	}
}

IServerSocket::RemoteInfo ServerSocketContext::GetClientInfo(uint64_t nConnId) {
	IServerSocket::RemoteInfo iInfo;
	memset(&iInfo, 0, sizeof(iInfo));

	auto it = _mConns.find(nConnId);
	if (it != _mConns.end()) {
		iInfo.nIP	= it->second.iAddr.sin_addr.s_addr;
		iInfo.nPort	= ntohs(it->second.iAddr.sin_port);
	}

	return move(iInfo);
}

void ServerSocketContext::__AcceptThread() {
	sockaddr_in	iAddr	= { 0 };
	socklen_t	nSize	= sizeof(iAddr);

#if !defined(_WIN32)
	static uint64_t nAllocId = 0;
#endif

	while (_nSocket != INVALID_SOCKET) {
		auto nAccept = accept(_nSocket, (sockaddr *)&iAddr, &nSize);
		if (nAccept != INVALID_SOCKET) {
		#if defined(_WIN32)
			uint64_t nConnId = (uint64_t)nAccept;
			u_long nFlags = 1;

			ioctlsocket(nAccept, FIONBIO, &nFlags);

			if (_tIO.fd_count >= FD_SETSIZE) {
				_pOwner->OnAccept(nConnId, ENet::TooMany);
				closesocket(nAccept);
				continue;
			}

			FD_SET(nAccept, &_tIO);
		#else
			uint64_t nConnId = nAllocId++;
			struct epoll_event iEvent = { 0 };

			iEvent.events = EPOLLIN | EPOLLET | EPOLLOUT;
			iEvent.data.fd = nAccept;
			iEvent.data.u64 = nConnId;

			if (epoll_ctl(_tIO, EPOLL_CTL_ADD, nAccept, &iEvent) != 0) {
				_pOwner->OnAccept(nConnId, errno);
				close(nAccept);
				continue;
			}
		#endif

			ConnectionInfo iConn;
			iConn.nSocket = nAccept;
			iConn.iAddr = iAddr;

			if (_mConns.find(nConnId) == _mConns.end())
				_mConns.insert(pair<uint64_t, ConnectionInfo>(nConnId, iConn));
			else
				_mConns[nConnId] = iConn;

			_pOwner->OnAccept(nConnId, 0);
		}

		this_thread::sleep_for(chrono::milliseconds(1));
	}
}

void ServerSocketContext::__IOThread() {
	char *	pTemp	= new char[256 * 1024];

#if !defined(_WIN32)
	struct epoll_event pEvents[4096];
#endif

	while (_nSocket != INVALID_SOCKET) {
	#if defined(_WIN32)
		fd_set iRead = _tIO;
		if (select(0, &iRead, NULL, NULL, NULL) <= 0) {
			this_thread::sleep_for(chrono::milliseconds(50));
			continue;
		}

		for (u_int n = 0; n < iRead.fd_count; ++n) {
			uint64_t	nConnId	= (uint64_t) iRead.fd_array[n];
			int32_t		nRecv	= sizeof(SocketData);

			memset(pTemp, 0, 256 * 1024);

			while (true) {
				int32_t nRead = (int32_t)recv(iRead.fd_array[n], pTemp + nRecv, 256 * 1024 - nRecv, MSG_DONTWAIT);
				if (nRead > 0) {
					nRecv += nRead;
					if (nRecv >= 256 * 1024) break;
				} else if (nRead < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
					break;
				} else {
					Close(nConnId, nRead == 0 ? ENet::Remote : ENet::BadData);
					break;
				}
			}

			if (nRecv > sizeof(SocketData)) {
				SocketData * pData = (SocketData *)pTemp;
				pData->nConnId = nConnId;
				pData->nSize = nRecv;
				pData->nUsed = 1;

				while (!_iBuffer.Write(pTemp, nRecv)) this_thread::sleep_for(chrono::nanoseconds(1));
			}
		}
	#else
		int nCount = epoll_wait(_tIO, pEvents, 4096, 50);

		for (int n = 0; n < nCount; ++n) {
			if (pEvents[n].events & EPOLLIN) {
				uint64_t nConnId = pEvents[n].data.u64;
				int nSocket = pEvents[n].data.fd;
				ssize_t nRecv = sizeof(SocketData);

				memset(pTemp, 0, 256 * 1024);

				while (true) {
					ssize_t nRead = recv(nSocket, pTemp + nRecv, 256 * 1024 - nRecv, MSG_DONTWAIT);
					if (nRead > 0) {
						nRecv += nRead;
						if (nRecv >= 256 * 1024) break;
					} else if (nRead < 0 && errno == EAGAIN) {
						break;
					} else {
						Close(nConnId, nRead == 0 ? ENet::Remote : ENet::BadData);
						break;
					}
				}

				if (nRecv > sizeof(SocketData)) {
					SocketData * pData = (SocketData *)pTemp;
					pData->nConnId = nConnId;
					pData->nSize = nRecv;
					pData->nUsed = 1;

					while (!_iBuffer.Write(pTemp, nRecv)) this_thread::sleep_for(chrono::nanoseconds(1));
				}
			}
		}
	#endif
	}

	delete[] pTemp;
}

ISocket::ISocket() : _pCtx(nullptr) {
	_pCtx = new SocketContext(this);
}

ISocket::~ISocket() {
	Close();
	if (_pGuard) delete _pGuard;
	if (_pCtx) delete _pCtx;
}

int ISocket::Connect(const string & sIP, int nPort, bool bAutoReconnect /* = false */) {
	if (sIP.empty() || nPort < 0) return ENet::BadParam;

	int n = _pCtx->Connect(sIP, nPort);
	if (n == ENet::Success && bAutoReconnect && !_pGuard) {
		_pGuard = new SocketGuard(this, sIP, nPort);
		_pGuard->Start();
	}

	return n;
}

bool ISocket::IsConnected() {
	return _pCtx->IsConnected();
}

void ISocket::Close() {
	_pCtx->Close(ENet::Local);
}

bool ISocket::Send(const char * pData, size_t nSize) {
	if (!pData || nSize < 0) return false;
	return _pCtx->Send(pData, nSize);
}

void ISocket::Breath() {
	_pCtx->Breath();
}

IServerSocket::IServerSocket() : _pCtx(nullptr) {
	_pCtx = new ServerSocketContext(this);
}

IServerSocket::~IServerSocket() {
	Shutdown();
	delete _pCtx;
}

int IServerSocket::Listen(const string & sIP, int nPort) {
	if (sIP.empty() || nPort < 0) return ENet::BadParam;
	return _pCtx->Listen(sIP, nPort);
}

bool IServerSocket::Send(uint64_t nConnId, const char * pData, size_t nSize) {
	if (!pData || nSize <= 0) return false;
	return _pCtx->Send(nConnId, pData, nSize);
}

void IServerSocket::Broadcast(const char * pData, size_t nSize) {
	if (!pData || nSize <= 0) return;
	_pCtx->Broadcast(pData, nSize);
}

void IServerSocket::Close(uint64_t nConnId) {
	_pCtx->Close(nConnId, ENet::Local);
}

void IServerSocket::Shutdown() {
	_pCtx->Shutdown();
}

IServerSocket::RemoteInfo IServerSocket::GetClientInfo(uint64_t nConnId) {
	return _pCtx->GetClientInfo(nConnId);
}

void IServerSocket::Breath() {
	_pCtx->Breath();
}

string IServerSocket::RemoteInfo::GetIP() const {
	int nPart1 = nIP & 0xFF;
	int nPart2 = (nIP >> 8) & 0xFF;
	int nPart3 = (nIP >> 16) & 0xFF;
	int nPart4 = (nIP >> 24) & 0xFF;
	char pBuf[16] = { 0 };

	snprintf(pBuf, 16, "%d.%d.%d.%d", nPart4, nPart3, nPart2, nPart1);
	return string(pBuf);
}

#if defined(_WIN32)
#undef FD_SETSIZE
#endif

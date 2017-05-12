#if defined(_WIN32)

#include	<Network.h>
#include	<Logger.h>

#define		FD_SETSIZE	4096
#include	<WinSock2.h>
#include	<WS2tcpip.h>
#pragma		comment(lib, "ws2_32.lib")

#include	<cstdlib>
#include	<cstring>
#include	<map>
#include	<thread>
#include	<vector>

#define		SOCKET_BUFSIZE	2097152

using namespace std;

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
	ISocket *		_pOwner;
	char *			_pReceived;
	SOCKET			_nSocket;
};

SocketContext::SocketContext(ISocket * pOwner)
	: _pOwner(pOwner)
	, _pReceived(new char[SOCKET_BUFSIZE])
	, _nSocket(INVALID_SOCKET) {
	WSADATA wOut;
	if (WSAStartup(MAKEWORD(2, 2), &wOut)) throw runtime_error("WinSock2 Startup failed!!!");
}

SocketContext::~SocketContext() {
	Close(ENet::Local);
	WSACleanup();
	delete[] _pReceived;
}

int SocketContext::Connect(const string & sIP, int nPort) {
	if (_nSocket != INVALID_SOCKET) return ENet::Running;
	if ((_nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) return WSAGetLastError();
	
	u_long nFlags = 1;
	if (ioctlsocket(_nSocket, FIONBIO, &nFlags) != 0) {
		closesocket(_nSocket);
		_nSocket = INVALID_SOCKET;
		return WSAGetLastError();
	}

	struct sockaddr_in iAddr;
	memset(&iAddr, 0, sizeof(iAddr));

	iAddr.sin_family	= AF_INET;
	iAddr.sin_port		= htons(nPort);

	if (0 >= inet_pton(AF_INET, sIP.c_str(), &iAddr.sin_addr)) {
		closesocket(_nSocket);
		_nSocket = INVALID_SOCKET;
		return ENet::BadParam;
	}

	if (connect(_nSocket, (sockaddr *)&iAddr, sizeof(sockaddr)) < 0) {
		int nErr = WSAGetLastError();
		if (nErr != WSAEWOULDBLOCK) return nErr;

		fd_set iSet;
		struct timeval iWait;

		iWait.tv_sec	= 3;
		iWait.tv_usec	= 0;

		FD_ZERO(&iSet);
		FD_SET(_nSocket, &iSet);

		if (select(0, 0, &iSet, 0, &iWait) <= 0) {
			closesocket(_nSocket);
			_nSocket = INVALID_SOCKET;
			return ENet::Timeout;
		}
	}

	_pOwner->OnConnected();
	return 0;
}

void SocketContext::Close(ENet::Close emCode) {
	if (_nSocket == INVALID_SOCKET) return;
	closesocket(_nSocket);
	_nSocket = INVALID_SOCKET;

	_pOwner->OnClose(emCode);
}

bool SocketContext::Send(const char * pData, size_t nSize) {
	if (_nSocket == INVALID_SOCKET) return false;

	char *	pSend	= (char *)pData;
	int		nSend	= 0;
	int		nLeft	= (int)nSize;

	while (true) {
		nSend = send(_nSocket, pSend, nLeft, 0);
		if (nSend < 0) {
			int nErr = WSAGetLastError();
			if (nErr == WSAEWOULDBLOCK) {
				Sleep(1);
			} else {
				return false;
			}
		} else if (nSend < nLeft) {
			nLeft -= nSend;
			pSend += nSend;
		} else if (nSend == nLeft) {
			return true;
		} else {
			return nLeft == 0;
		}
	}
}

void SocketContext::Breath() {
	if (_nSocket == INVALID_SOCKET) return;

	memset(_pReceived, 0, SOCKET_BUFSIZE);
	int nReaded = 0;

	while (true) {
		int nRecv = recv(_nSocket, _pReceived + nReaded, SOCKET_BUFSIZE - nReaded, 0);
		if (nRecv < 0) {
			int nErr = WSAGetLastError();
			if (nErr == WSAEWOULDBLOCK) {
				break;
			} else {
				Close(ENet::BadData);
				break;
			}
		} else if (nRecv == 0) {
			Close(ENet::Remote);
			break;
		} else {
			nReaded += nRecv;
		}
	}

	if (nReaded > 0) _pOwner->OnReceive(_pReceived, nReaded);
}

class ServerSocketContext {
	typedef map<uint64_t, Connection *> ConnectionMap;

public:
	ServerSocketContext(IServerSocket * pOwner);
	virtual ~ServerSocketContext();

	int		Listen(const string & sIP, int nPort);
	bool	Send(Connection * pConn, const char * pData, size_t nSize);
	void	Broadcast(const char * pData, size_t nSize);
	void	Close(Connection * pConn, ENet::Close emCode);
	void	Shutdown();
	void	Breath();

	Connection *	Find(uint64_t nConnId);

private:
	IServerSocket *			_pOwner;
	char *					_pReceived;
	SOCKET					_nSocket;
	ConnectionMap			_mConns;
	ConnectionMap			_mSocket2Conns;
	fd_set					_tIO;
};

ServerSocketContext::ServerSocketContext(IServerSocket * pOwner)
	: _pOwner(pOwner)
	, _pReceived(new char[SOCKET_BUFSIZE])
	, _nSocket(INVALID_SOCKET)
	, _mConns()
	, _mSocket2Conns()
	, _tIO() {
	WSADATA wOut;
	if (WSAStartup(MAKEWORD(2, 2), &wOut)) throw runtime_error("WinSock2 Startup failed!!!");
}

ServerSocketContext::~ServerSocketContext() {
	Shutdown();
	WSACleanup();
	delete[] _pReceived;
}

int ServerSocketContext::Listen(const string & sIP, int nPort) {
	if (_nSocket != INVALID_SOCKET) return ENet::Running;
	if ((_nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) return WSAGetLastError();

	u_long nFlags = 1;
	if (ioctlsocket(_nSocket, FIONBIO, &nFlags) != 0) {
		closesocket(_nSocket);
		_nSocket = INVALID_SOCKET;
		return WSAGetLastError();
	}

	FD_ZERO(&_tIO);

	struct sockaddr_in iAddr;
	memset(&iAddr, 0, sizeof(iAddr));

	iAddr.sin_family	= AF_INET;
	iAddr.sin_port		= htons(nPort);

	if (0 >= inet_pton(AF_INET, sIP.c_str(), &iAddr.sin_addr)) {
		closesocket(_nSocket);
		_nSocket = INVALID_SOCKET;
		return ENet::BadParam;
	}

	if (::bind(_nSocket, (sockaddr *)&iAddr, sizeof(sockaddr)) < 0 || ::listen(_nSocket, 512) < 0) {
		closesocket(_nSocket);
		_nSocket = INVALID_SOCKET;
		return WSAGetLastError();
	}

	return 0;
}

bool ServerSocketContext::Send(Connection * pConn, const char * pData, size_t nSize) {
	if (!pConn) return false;

	SOCKET	nSocket	= (SOCKET)pConn->nSocket;
	char *	pSend	= (char *)pData;
	int		nSend	= 0;
	int		nLeft	= (int)nSize;

	while (true) {
		nSend = send(nSocket, pSend, nLeft, 0);
		if (nSend < 0) {
			int nErr = WSAGetLastError();
			if (nErr == WSAEWOULDBLOCK) {
				Sleep(1);
			} else {
				return false;
			}
		} else if (nSend < nLeft) {
			nLeft -= nSend;
			pSend += nSend;
		} else if (nSend == nLeft) {
			return true;
		} else {
			return nLeft == 0;
		}
	}
}

void ServerSocketContext::Broadcast(const char * pData, size_t nSize) {
	for (auto & kv : _mConns) {
		SOCKET	nSocket	= (SOCKET)kv.second->nSocket;
		char *	pSend	= (char *)pData;
		int		nSend	= 0;
		int		nLeft	= (int)nSize;

		while (true) {
			nSend = send(nSocket, pSend, nLeft, 0);
			if (nSend < 0) {
				int nErr = WSAGetLastError();
				if (nErr == WSAEWOULDBLOCK) {
					Sleep(1);
				} else {
					break;
				}
			} else if (nSend < nLeft) {
				nLeft -= nSend;
				pSend += nSend;
			} else {
				break;
			}
		}
	}
}

void ServerSocketContext::Close(Connection * pConn, ENet::Close emCode) {
	if (!pConn) return;

	SOCKET nSocket = (SOCKET)pConn->nSocket;
	uint64_t nConnId = pConn->nId;
	
	_pOwner->OnClose(pConn, emCode);
	FD_CLR(nSocket, &_tIO);
	closesocket(nSocket);
	delete pConn;

	_mConns.erase(nConnId);
	_mSocket2Conns.erase((uint64_t)nSocket);
}

void ServerSocketContext::Shutdown() {
	if (_nSocket == INVALID_SOCKET) return;

	for (auto & kv : _mConns) {
		Connection * pConn = kv.second;
		_pOwner->OnClose(pConn, ENet::Local);
		closesocket((SOCKET)pConn->nSocket);
		delete pConn;
	}

	FD_ZERO(&_tIO);
	_mConns.clear();
	_mSocket2Conns.clear();

	closesocket(_nSocket);
	_nSocket = INVALID_SOCKET;

	_pOwner->OnShutdown();
}

void ServerSocketContext::Breath() {
	if (_nSocket == INVALID_SOCKET) return;
	static fd_set iRead;
	static uint64_t nAllocId = 0;
	static struct timeval iWait = { 0, 1 };

	sockaddr_in iAddr;
	int nSizeOfAddr = sizeof(iAddr);
	char pAddr[128];

	while (true) {
		SOCKET nAccept = accept(_nSocket, (sockaddr *)&iAddr, &nSizeOfAddr);
		if (nAccept == INVALID_SOCKET) break;

		inet_ntop(AF_INET, &iAddr, pAddr, 128);

		u_long nFlag = 1;
		if (ioctlsocket(nAccept, FIONBIO, &nFlag) != 0) {			
			LOG_WARN("Failed accept client [%s] while setting non-block!!!", pAddr);
			continue;
		}

		if (_tIO.fd_count >= FD_SETSIZE) {
			LOG_WARN("Failed accept client [%s] because >= FD_SETSIZE(%d)", pAddr, FD_SETSIZE);
			continue;
		}

		FD_SET(nAccept, &_tIO);

		uint64_t nConnId = nAllocId + 1;
		nAllocId++;

		Connection * pConn	= new Connection;
		pConn->nId		= nConnId;
		pConn->nSocket	= (int)nAccept;
		pConn->nIP		= iAddr.sin_addr.s_addr;
		pConn->nPort	= iAddr.sin_port;
		pConn->pData	= nullptr;

		_mConns[nConnId] = pConn;
		_mSocket2Conns[(uint64_t)nAccept] = pConn;

		_pOwner->OnAccept(pConn);
	}

	memcpy(&iRead, &_tIO, sizeof(_tIO));

	if (select(0, &iRead, 0, 0, &iWait) <= 0) return;

	for (u_int n = 0; n < iRead.fd_count; ++n) {
		SOCKET nSocket = iRead.fd_array[n];

		auto it = _mSocket2Conns.find((uint64_t)nSocket);
		if (it == _mSocket2Conns.end()) continue;

		int nReaded = 0;
		int nRecv = 0;

		memset(_pReceived, 0, SOCKET_BUFSIZE);

		while (true) {
			nRecv = recv(nSocket, _pReceived + nReaded, SOCKET_BUFSIZE - nReaded, 0);
			if (nRecv > 0) {
				nReaded += nRecv;
				if (nReaded >= SOCKET_BUFSIZE) {
					_pOwner->OnReceive(it->second, _pReceived, nReaded);
					memset(_pReceived, 0, SOCKET_BUFSIZE);
					nReaded = 0;
				}
			} else if (nRecv < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
				if (nReaded > 0) _pOwner->OnReceive(it->second, _pReceived, nReaded);
				break;
			} else {
				if (nReaded > 0) _pOwner->OnReceive(it->second, _pReceived, nReaded);
				Close(it->second, nRecv == 0 ? ENet::Remote : ENet::BadData);
				break;
			}
		}
	}
}

Connection * ServerSocketContext::Find(uint64_t nConnId) {
	auto it = _mConns.find(nConnId);
	if (it == _mConns.end()) return nullptr;
	return it->second;
}

class SocketGuard {
public:
	SocketGuard(ISocket * p, const string & sHost, int nPort);
	virtual ~SocketGuard();

	void	Start();

private:
	ISocket *	_p;
	string		_sHost;
	int			_nPort;
	thread *	_pWorker;
	bool		_bRunning;
};

SocketGuard::SocketGuard(ISocket * p, const string & sHost, int nPort)
	: _p(p), _sHost(sHost), _nPort(nPort), _pWorker(nullptr), _bRunning(false) {}

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
				if (n != ENet::Success) LOG_WARN("Try to reconnect [%s:%d] ... %d", _sHost.c_str(), _nPort, n);
			}

			this_thread::sleep_for(chrono::seconds(1));
		}
	});
}

class NetworkBreather {
public:
	NetworkBreather() {}

	static NetworkBreather & Get();

	void	Add(ISocket * p);
	void	Add(IServerSocket * p);
	void	Del(ISocket * p);
	void	Del(IServerSocket * p);
	
	void	Breath();

private:
	vector<ISocket *>		_vClients;
	vector<IServerSocket *>	_vServers;
};

NetworkBreather & NetworkBreather::Get() {
	static unique_ptr<NetworkBreather> pIns;
	if (!pIns) pIns.reset(new NetworkBreather);
	return *pIns;
}

void NetworkBreather::Add(ISocket * p) {
	for (auto pClient : _vClients) {
		if (pClient == p) return;
	}

	_vClients.push_back(p);
}

void NetworkBreather::Add(IServerSocket * p) {
	for (auto pServer : _vServers) {
		if (pServer == p) return;
	}

	_vServers.push_back(p);
}

void NetworkBreather::Del(ISocket * p) {
	auto it = find(_vClients.begin(), _vClients.end(), p);
	if (it != _vClients.end()) _vClients.erase(it);
}

void NetworkBreather::Del(IServerSocket * p) {
	auto it = find(_vServers.begin(), _vServers.end(), p);
	if (it != _vServers.end()) _vServers.erase(it);
}

void NetworkBreather::Breath() {
	for (auto p : _vClients) p->Breath();
	for (auto p : _vServers) p->Breath();
}

void AutoNetworkBreath() {
	NetworkBreather::Get().Breath();
}

ISocket::ISocket() : _pCtx(nullptr), _pGuard(nullptr) {
	_pCtx = new SocketContext(this);
	NetworkBreather::Get().Add(this);
}

ISocket::~ISocket() {
	NetworkBreather::Get().Del(this);
	Close();
	if (_pGuard) delete _pGuard;
	if (_pCtx) delete _pCtx;
}

int ISocket::Connect(const std::string & sIP, int nPort, bool bAutoReconnect /* = false */) {
	if (sIP.empty() || nPort < 0) return ENet::BadParam;
	
	int n = _pCtx->Connect(sIP, nPort);
	if (n == 0 && bAutoReconnect && !_pGuard) {
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

	if (_pGuard) {
		delete _pGuard;
		_pGuard = nullptr;
	}
}

bool ISocket::Send(const char * pData, size_t nSize) {
	if (!pData || nSize <= 0) return false;
	return _pCtx->Send(pData, nSize);
}

void ISocket::Breath() {
	_pCtx->Breath();
}

IServerSocket::IServerSocket() : _pCtx(nullptr) {
	_pCtx = new ServerSocketContext(this);
	NetworkBreather::Get().Add(this);
}

IServerSocket::~IServerSocket() {
	NetworkBreather::Get().Del(this);
	Shutdown();
	if (_pCtx) delete _pCtx;
}

int IServerSocket::Listen(const std::string & sIP, int nPort) {
	if (sIP.empty() || nPort < 0) return ENet::BadParam;
	return _pCtx->Listen(sIP, nPort);
}

bool IServerSocket::Send(Connection * pConn, const char * pData, size_t nSize) {
	if (!pData || nSize <= 0) return false;
	return _pCtx->Send(pConn, pData, nSize);
}

void IServerSocket::Broadcast(const char * pData, size_t nSize) {
	if (!pData || nSize <= 0) return;
	_pCtx->Broadcast(pData, nSize);
}

void IServerSocket::Close(Connection * pConn) {
	_pCtx->Close(pConn, ENet::Local);
}

void IServerSocket::Shutdown() {
	_pCtx->Shutdown();
}

void IServerSocket::Breath() {
	_pCtx->Breath();
}

Connection * IServerSocket::Find(uint64_t nConnId) {
	return _pCtx->Find(nConnId);
}

string Connection::IP() const {
	char pAddr[16];

	int n1 = (nIP & 0xFF);
	int n2 = (nIP >> 8) & 0xFF;
	int n3 = (nIP >> 16) & 0xFF;
	int n4 = (nIP >> 24) & 0xFF;

	snprintf(pAddr, 16, "%d.%d.%d.%d", n1, n2, n3, n4);
	return string(pAddr);
}

#endif

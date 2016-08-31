#ifndef		__ENGINE_NETWORK_H_INCLUDED__
#define		__ENGINE_NETWORK_H_INCLUDED__

#include	<cstdint>
#include	<string>

namespace ENet {

	/**
	 * Socket status or error code.
	 **/
	enum Error {
		Success = 0,
		None = 0,
		BadParam = -65536,
		Closed,
		Running,
		Alloc,
		Epoll,
		Timeout,
		TooMany
	};

	/**
	 * Close reason.
	 **/
	enum Close {
		Local,
		Remote,
		BadData
	};
}

/**
 * TCP client side.
 **/
class ISocket {
public:
	ISocket();
	virtual ~ISocket();

	/**
	 * Connect to TCP server.
	 *
	 * \param	sIP		IP address of remote server.
	 * \param	nPort	Port to connect to.
	 * \param	bAutoReconnect	Set 'true' to enable auto-reconnect.
	 * \return	Connect status. See ENet::Error for detail information.
	 **/
	int Connect(const std::string & sIP, int nPort, bool bAutoReconnect = false);

	/**
	 * Check connection status.
	 **/
	bool IsConnected();

	/**
	 * Disconnect from server.
	 **/
	void Close();

	/**
	 * Send data to server.
	 *
	 * \param	pData	Pointer to data buffer.
	 * \param	nSize	Size of data to be sent.
	 * \return	Failed to send data will return false.
	 **/
	bool Send(const char * pData, size_t nSize);

	/**
	 * Process all received data at once. This may invoke OnReceive() many times.
	 * NOTE : Except using Application, you should call this in you main event loop.
	 **/
	void Breath();

	/**
	 * Action to do after connected to server.
	 **/
	virtual void OnConnected() {}

	/**
	 * Action to do when received message from server.
	 *
	 * \param	pData	Pointer to message.
	 * \param	nSize	Size of this message in bytes.
	 **/
	virtual void OnReceive(char * pData, size_t nSize) = 0;

	/**
	 * Action to do when disconnect from server. Detail info will output to logs.
	 *
	 * \param	emCode	Close reason. See ENet::Close for detail information.
	 **/
	virtual void OnClose(ENet::Close emCode) {}

private:
	class SocketContext *	_pCtx;
	class SocketGuard *		_pGuard;
};

/**
 * TCP server side.
 **/
class IServerSocket {
public:
	struct RemoteInfo {
		uint32_t	nIP;
		int			nPort;

		std::string GetIP() const;
	};

public:
	IServerSocket();
	virtual ~IServerSocket();

	/**
	 * Listen for connections.
	 *
	 * \param	sIP		Listen ip.
	 * \param	nPort	Port to listen on.
	 * \return	Listen status. See ENet::Error.
	 **/
	int Listen(const std::string & sIP, int nPort);

	/**
	 * Send data to special client.
	 *
	 * \param	nConnId	Client identifier generated by OnAccept();
	 * \param	pData	Pointer to data buffer.
	 * \param	nSize	Size of data.
	 * \return	if successfully send.
	 **/
	bool Send(uint64_t nConnId, const char * pData, size_t nSize);

	/**
	 * Broadcast message to all connected clients.
	 *
	 * \param	pData	Pointer to data buffer.
	 * \param	nSize	Size of data.
	 **/
	void Broadcast(const char * pData, size_t nSize);

	/**
	 * Manually close a connection with special client.
	 *
	 * \param	nConnId	Client identifier generated by OnAccept().
	 **/
	void Close(uint64_t nConnId);

	/**
	 * Shutdown this server.
	 **/
	void Shutdown();

	/**
	 * Get client info.
	 *
	 * \param	nConnId	Client identifier generated by OnAccept().
	 **/
	RemoteInfo GetClientInfo(uint64_t nConnId);

	/**
	 * Process all received data at once. This may invoke OnReceive() many times.
	 * NOTE : Except using Application, you should call this in you main event loop.
	 **/
	void Breath();

	/**
	 * Invoked after a client try to connect to this server.
	 *
	 * \param	nConnId	Client identifier.
	 * \param	iInfo	Remote information.
	 **/
	virtual void OnAccept(uint64_t nConnId, const RemoteInfo & iInfo) {}

	/**
	 * Invoked by OnTick(). Jobs to with message received from client.
	 *
	 * \param	nConnId	Client identifier.
	 * \param	pData	Pointer to message data buffer.
	 * \param	nSize	Message size.
	 **/
	virtual void OnReceive(uint64_t nConnId, char * pData, size_t nSize) = 0;

	/**
	 * Invoked after a client disconnect with this server.
	 *
	 * \param	nConnId	Client identifier.
	 * \param	emCode	Close reason. See ENet::Close.
	 **/
	virtual void OnClose(uint64_t nConnId, ENet::Close emCode) {}

	/**
	 * Action to do before server shutdown.
	 **/
	virtual void OnShutdown() {}

private:
	class ServerSocketContext *	_pCtx;
};

#endif//!	__ENGINE_NETWORK_H_INCLUDED__
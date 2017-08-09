#include "stdafx.h"
#include "MT_AsyncSocket.h"


CMT_AsyncSocket::CMT_AsyncSocket()
{
	m_WarrantFlag = FALSE;
	UserOnReceive = NULL;
}


CMT_AsyncSocket::~CMT_AsyncSocket()
{
}

int CMT_AsyncSocket::AsyncSocketInit(unsigned int nSocketPort, int nSocketType /*= SOCK_STREAM*/, long lEvent/* = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/, LPCTSTR lpszSocketAddress/* = NULL*/)
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(_T("Failed to Initialize Sockets"), MB_OK | MB_ICONSTOP);
		return -1;
	}
	if (!Create(nSocketPort, nSocketType, lEvent, lpszSocketAddress))
	{
		int nErrorCode = GetLastError();
		ErrorWarnOfCreate(nErrorCode);
		return -1;
	}

	return 0;
}
void CMT_AsyncSocket::OnReceive(int nErrorCode)
{
	if (NULL != UserOnReceive)
	{
		(*UserOnReceive)(nErrorCode);
	}
	CAsyncSocket::OnReceive(nErrorCode);
}
void CMT_AsyncSocket::ErrorWarnOfCreate(int nErrorCode)
{
	switch (nErrorCode)
	{
	case WSANOTINITIALISED:
		AfxMessageBox(_T("A successful AfxSocketInit must occur before using this API."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENETDOWN:
		AfxMessageBox(_T("The Windows Sockets implementation detected that the network subsystem failed."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEAFNOSUPPORT:
		AfxMessageBox(_T("The specified address family is not supported."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEINPROGRESS:
		AfxMessageBox(_T("A blocking Windows Sockets operation is in progress."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEMFILE:
		AfxMessageBox(_T("No more file descriptors are available."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENOBUFS:
		AfxMessageBox(_T("No buffer space is available. The socket cannot be created."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEPROTONOSUPPORT:
		AfxMessageBox(_T("The specified protocol is not supported."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEPROTOTYPE:
		AfxMessageBox(_T("The specified protocol is the wrong type for this socket."), MB_OK | MB_ICONSTOP);
		break;
	case WSAESOCKTNOSUPPORT:
		AfxMessageBox(_T("The specified socket type is not supported in this address family."), MB_OK | MB_ICONSTOP);
		break;
	}
}

void CMT_AsyncSocket::ErrorWarnOfSend(int nErrorCode)
{
	switch (nErrorCode)
	{
	case WSANOTINITIALISED:
		AfxMessageBox(_T("A successful AfxSocketInit must occur before using this API."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENETDOWN:
		AfxMessageBox(_T("The Windows Sockets implementation detected that the network subsystem failed."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEACCES:
		AfxMessageBox(_T("The requested address is a broadcast address, but the appropriate flag was not set."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEINPROGRESS:
		AfxMessageBox(_T("A blocking Windows Sockets operation is in progress."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEFAULT:
		AfxMessageBox(_T("The lpBuf argument is not in a valid part of the user address space."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENETRESET:
		AfxMessageBox(_T("The connection must be reset because the Windows Sockets implementation dropped it."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENOBUFS:
		AfxMessageBox(_T("The Windows Sockets implementation reports a buffer deadlock."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENOTCONN:
		AfxMessageBox(_T("The socket is not connected."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENOTSOCK:
		AfxMessageBox(_T("The descriptor is not a socket."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEOPNOTSUPP:
		AfxMessageBox(_T("MSG_OOB was specified, but the socket is not of type SOCK_STREAM."), MB_OK | MB_ICONSTOP);
		break;
	case WSAESHUTDOWN:
		AfxMessageBox(_T("The socket has been shut down; it is not possible to call ReceiveFrom on a socket after ShutDown has been invoked with nHow set to 0 or 2."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEWOULDBLOCK:
		AfxMessageBox(_T("The socket is marked as nonblocking and the ReceiveFrom operation would block."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEMSGSIZE:
		AfxMessageBox(_T("The socket is of type SOCK_DGRAM, and the datagram is larger than the maximum supported by the Windows Sockets implementation."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEINVAL:
		AfxMessageBox(_T("The socket has not been bound with Bind."), MB_OK | MB_ICONSTOP);
		break;
	case WSAECONNABORTED:
		AfxMessageBox(_T("The virtual circuit was aborted due to timeout or other failure."), MB_OK | MB_ICONSTOP);
		break;
	case WSAECONNRESET:
		AfxMessageBox(_T("The virtual circuit was reset by the remote side."), MB_OK | MB_ICONSTOP);
		break;
	}
}
void CMT_AsyncSocket::ErrorWarnOfSendTo(int nErrorCode)
{
	switch (nErrorCode)
	{
	case WSANOTINITIALISED:
		AfxMessageBox(_T("A successful AfxSocketInit must occur before using this API."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENETDOWN:
		AfxMessageBox(_T("The Windows Sockets implementation detected that the network subsystem failed."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENOTCONN:
		AfxMessageBox(_T("The socket is not connected (SOCK_STREAM only)."), MB_OK | MB_ICONSTOP);
		break;
	case WSAENOTSOCK:
		AfxMessageBox(_T("The descriptor is not a socket."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEOPNOTSUPP:
		AfxMessageBox(_T("MSG_OOB was specified, but the socket is not of type SOCK_STREAM."), MB_OK | MB_ICONSTOP);
		break;
	case WSAESHUTDOWN:
		AfxMessageBox(_T("The socket has been shut down; it is not possible to call ReceiveFrom on a socket after ShutDown has been invoked with nHow set to 0 or 2."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEWOULDBLOCK:
		AfxMessageBox(_T("The socket is marked as nonblocking and the ReceiveFrom operation would block."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEMSGSIZE:
		AfxMessageBox(_T("The datagram was too large to fit into the specified buffer and was truncated."), MB_OK | MB_ICONSTOP);
		break;
	case WSAEINVAL:
		AfxMessageBox(_T("The socket has not been bound with Bind."), MB_OK | MB_ICONSTOP);
		break;
	case WSAECONNABORTED:
		AfxMessageBox(_T("The virtual circuit was aborted due to timeout or other failure."), MB_OK | MB_ICONSTOP);
		break;
	case WSAECONNRESET:
		AfxMessageBox(_T("The virtual circuit was reset by the remote side."), MB_OK | MB_ICONSTOP);
		break;
	}
}

void CMT_AsyncSocket::ErrorWarnOfReceiveFrom(int nErrorCode)
{
	switch (nErrorCode)
	{
	case WSANOTINITIALISED:
		AfxMessageBox(_T("A successful AfxSocketInit must occur before using this API."));
		break;
	case WSAENETDOWN:
		AfxMessageBox(_T("The Windows Sockets implementation detected that the network subsystem failed."));
		break;
	case WSAEFAULT:
		AfxMessageBox(_T("The lpSockAddrLen argument was invalid: the lpSockAddr buffer was too small to accommodate the peer address."));
		break;
	case WSAEINPROGRESS:
		AfxMessageBox(_T("A blocking Windows Sockets operation is in progress."));
		break;
	case WSAEINVAL:
		AfxMessageBox(_T("The socket has not been bound with Bind."));
		break;
	case WSAENOTCONN:
		AfxMessageBox(_T("The socket is not connected (SOCK_STREAM only)."));
		break;
	case WSAENOTSOCK:
		AfxMessageBox(_T("The descriptor is not a socket."));
		break;
	case WSAEOPNOTSUPP:
		AfxMessageBox(_T("MSG_OOB was specified, but the socket is not of type SOCK_STREAM."));
		break;
	case WSAESHUTDOWN:
		AfxMessageBox(_T("The socket has been shut down; it is not possible to call ReceiveFrom on a socket after ShutDown has been invoked with nHow set to 0 or 2."));
		break;
	case WSAEWOULDBLOCK:
		AfxMessageBox(_T("The socket is marked as nonblocking and the ReceiveFrom operation would block."));
		break;
	case WSAEMSGSIZE:
		AfxMessageBox(_T("The datagram was too large to fit into the specified buffer and was truncated."));
		break;
	case WSAECONNABORTED:
		AfxMessageBox(_T("The virtual circuit was aborted due to timeout or other failure."));
		break;
	case WSAECONNRESET:
		AfxMessageBox(_T("The virtual circuit was reset by the remote side."));
		break;
	default:
		AfxMessageBox(_T("ReceiveFrom have an error!"));
		break;
	}
	exit(-1);
}

void CMT_AsyncSocket::ErrorWarnOfReceive(int nErrorCode)
{
	switch (nErrorCode)
	{
	case WSANOTINITIALISED:
		AfxMessageBox(_T("A successful AfxSocketInit must occur before using this API."));
		break;
	case WSAENETDOWN:
		AfxMessageBox(_T("The Windows Sockets implementation detected that the network subsystem failed."));
		break;
	case WSAEFAULT:
		AfxMessageBox(_T("The lpSockAddrLen argument was invalid: the lpSockAddr buffer was too small to accommodate the peer address."));
		break;
	case WSAEINPROGRESS:
		AfxMessageBox(_T("A blocking Windows Sockets operation is in progress."));
		break;
	case WSAEINVAL:
		AfxMessageBox(_T("The socket has not been bound with Bind."));
		break;
	case WSAENOTCONN:
		AfxMessageBox(_T("The socket is not connected (SOCK_STREAM only)."));
		break;
	case WSAENOTSOCK:
		AfxMessageBox(_T("The descriptor is not a socket."));
		break;
	case WSAEOPNOTSUPP:
		AfxMessageBox(_T("MSG_OOB was specified, but the socket is not of type SOCK_STREAM."));
		break;
	case WSAESHUTDOWN:
		AfxMessageBox(_T("The socket has been shut down; it is not possible to call ReceiveFrom on a socket after ShutDown has been invoked with nHow set to 0 or 2."));
		break;
	case WSAEWOULDBLOCK:
		AfxMessageBox(_T("The socket is marked as nonblocking and the ReceiveFrom operation would block."));
		break;
	case WSAEMSGSIZE:
		AfxMessageBox(_T("The datagram was too large to fit into the specified buffer and was truncated."));
		break;
	case WSAECONNABORTED:
		AfxMessageBox(_T("The virtual circuit was aborted due to timeout or other failure."));
		break;
	case WSAECONNRESET:
		AfxMessageBox(_T("The virtual circuit was reset by the remote side."));
		break;
	}
}

void CMT_AsyncSocket::ErrorWarnOfListen(int nErrorCode)
{
	switch (nErrorCode)
	{
	case WSANOTINITIALISED:
		AfxMessageBox(_T("A successful AfxSocketInit must occur before using this API."));
		break;
	case WSAENETDOWN:
		AfxMessageBox(_T("The Windows Sockets implementation detected that the network subsystem failed."));
		break;
	case WSAEADDRINUSE:
		AfxMessageBox(_T("An attempt has been made to listen on an address in use."));
		break;
	case WSAEINPROGRESS:
		AfxMessageBox(_T("A blocking Windows Sockets operation is in progress."));
		break;
	case WSAEINVAL:
		AfxMessageBox(_T("The socket has not been bound with Bind or is already connected."));
		break;
	case WSAEISCONN:
		AfxMessageBox(_T("The socket is already connected."));
		break;
	case WSAEMFILE:
		AfxMessageBox(_T("No more file descriptors are available."));
		break;
	case WSAENOBUFS:
		AfxMessageBox(_T("No buffer space is available."));
		break;
	case WSAENOTSOCK:
		AfxMessageBox(_T("The descriptor is not a socket."));
		break;
	case WSAEOPNOTSUPP:
		AfxMessageBox(_T("The referenced socket is not of a type that supports the Listen operation."));
		break;
	}
}
void CMT_AsyncSocket::ErrorWarnOfAccept(int nErrorCode)
{
	switch (nErrorCode)
	{
	case WSANOTINITIALISED:
		AfxMessageBox(_T("A successful AfxSocketInit must occur before using this API."));
		break;
	case WSAENETDOWN:
		AfxMessageBox(_T("The Windows Sockets implementation detected that the network subsystem failed."));
		break;
	case WSAEFAULT:
		AfxMessageBox(_T("The lpSockAddrLen argument is too small (less than the size of a SOCKADDR structure)."));
		break;
	case WSAEINPROGRESS:
		AfxMessageBox(_T("A blocking Windows Sockets operation is in progress."));
		break;
	case WSAEINVAL:
		AfxMessageBox(_T("Listen was not invoked prior to accept."));
		break;
	case WSAEMFILE:
		AfxMessageBox(_T("The queue is empty upon entry to accept and there are no descriptors available."));
		break;
	case WSAENOBUFS:
		AfxMessageBox(_T("No buffer space is available."));
		break;
	case WSAENOTSOCK:
		AfxMessageBox(_T("The descriptor is not a socket."));
		break;
	case WSAEOPNOTSUPP:
		AfxMessageBox(_T("The referenced socket is not a type that supports connection-oriented service."));
		break;
	case WSAEWOULDBLOCK:
		AfxMessageBox(_T("The socket is marked as nonblocking and no connections are present to be accepted."));
		break;
	}
}

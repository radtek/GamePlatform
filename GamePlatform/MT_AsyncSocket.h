#pragma once
#include "afxsock.h"
class CMT_AsyncSocket :
	public CAsyncSocket
{
public:
	CMT_AsyncSocket();
	~CMT_AsyncSocket();
	bool m_WarrantFlag;
	void *UserObject;
	int AsyncSocketInit(unsigned int nSocketPort = 0, int nSocketType = SOCK_STREAM, long lEvent = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE, LPCTSTR lpszSocketAddress = NULL);
	void(*UserOnReceive)(LPVOID pParam, int nErrorCode);

	void ErrorWarnOfCreate(int nErrorCode);
	void ErrorWarnOfListen(int nErrorCode);
	void ErrorWarnOfAccept(int nErrorCode);

	void ErrorWarnOfSend(int nErrorCode);
	void ErrorWarnOfSendTo(int nErrorCode);

	void ErrorWarnOfReceive(int nErrorCode);
	void ErrorWarnOfReceiveFrom(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
};


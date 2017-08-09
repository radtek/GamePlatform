#pragma once
#include "MT_AsyncSocket.h"
class CConnectToAttitudeSource :
	public CMT_AsyncSocket
{
public:
	CConnectToAttitudeSource();
	~CConnectToAttitudeSource();

	virtual void OnReceive(int nErrorCode);
};


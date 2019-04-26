#include "stdafx.h"
#include "ConnectToController.h"


CConnectToController::CConnectToController()
{
	TCHAR tControllerIP[17] = TEXT("192.168.0.125");
	_tcscpy_s(m_tcaControllerIP,tControllerIP);
	m_nControllerPort = 5000;
	
	memset(&m_preDataToDof, 0, sizeof(m_preDataToDof));
	memset(&m_sToDOFBuf,0,sizeof(m_sToDOFBuf));
	memset(&m_sDataFromMainControlToDof, 0, sizeof(m_sDataFromMainControlToDof));
	memset(&m_sReturnedDataFromDOF, 0, sizeof(m_sReturnedDataFromDOF));
	memset(m_tcaControllerIP, 0, sizeof(TCHAR)*17);
	m_sToDOFBuf.nCheckID = 55;
	m_sDataFromMainControlToDof.nCheckID = 55;
}


CConnectToController::~CConnectToController()
{
}

void CConnectToController::OnReceive(int nErrorCode)
{
	int t_nRet=0;
	unsigned char returnedDataFromNet[300];
	CString remoteIp;
	UINT remotePort;
	t_nRet = ReceiveFrom(&returnedDataFromNet, sizeof(returnedDataFromNet), remoteIp, remotePort, 0);
	if (SOCKET_ERROR == t_nRet)
	{
		ErrorWarnOfReceiveFrom(GetLastError());
	}
	else if (sizeof(DataToHost) == t_nRet)
	{
		memcpy(&m_sReturnedDataFromDOF, &returnedDataFromNet, sizeof(m_sReturnedDataFromDOF));
	}
	else if ((10000 == remotePort) && (sizeof(DataToDOF) == t_nRet) && (0==remoteIp.Compare(TEXT("192.168.0.130"))))
	{
		memcpy(&m_sDataFromMainControlToDof, &returnedDataFromNet, sizeof(m_sDataFromMainControlToDof));
		//lost part data
	}
	else
	{

	}
	CAsyncSocket::OnReceive(nErrorCode);
}
int CConnectToController::DOF_UpToMedian()
{
	m_sToDOFBuf.nCmd = S_CMD_Work;
	for (int i = 0; i<6; i++)
	{
		m_sToDOFBuf.DOFs[i] = 0.0f;
	}
	for (int i = 0; i<3; i++)
	{
		m_sToDOFBuf.Vxyz[i] = 0.0f;
		m_sToDOFBuf.Axyz[i] = 0.0f;
	}

	int t_timing = 0;
	while ((dof_neutral != m_sReturnedDataFromDOF.nDOFStatus) && (dof_working != m_sReturnedDataFromDOF.nDOFStatus))
	{
		SendTo(&m_sToDOFBuf, sizeof(m_sToDOFBuf),m_nControllerPort,/*L"192.168.0.125"*/m_tcaControllerIP);
		Sleep(10);
		t_timing++;
		if (t_timing >= m_MaxDelay)
		{
			AfxMessageBox(TEXT("DOF_UpToMedian failed!\r\nPlease check the manual!"));
			exit(-1);
		}
	} 

	return 0;
}


int CConnectToController::DOF_ToRun()
{
	m_sToDOFBuf.nCmd = S_CMD_RUN;
	for (int i = 0; i<6; i++)
	{
		m_sToDOFBuf.DOFs[i] = 0.0f;
	}
	for (int i = 0; i<3; i++)
	{
		m_sToDOFBuf.Vxyz[i] = 0.0f;
		m_sToDOFBuf.Axyz[i] = 0.0f;
	}

	int t_timing = 0;
	while (dof_working != m_sReturnedDataFromDOF.nDOFStatus)
	{
		SendTo(&m_sToDOFBuf, sizeof(m_sToDOFBuf), m_nControllerPort, m_tcaControllerIP);
		Sleep(10);
		t_timing++;
		if (t_timing >= m_MaxDelay)
		{
			AfxMessageBox(TEXT("DOF_ToRun failed!\r\nPlease check the manual!"));
			exit(-1);
		}
	} 

	return 0;
}


int CConnectToController::DOF_ToMedian()
{
	m_sToDOFBuf.nCmd = S_CMD_Back2MID;
	for (int i = 0; i<6; i++)
	{
		m_sToDOFBuf.DOFs[i] = 0.0f;
	}
	for (int i = 0; i<3; i++)
	{
		m_sToDOFBuf.Vxyz[i] = 0.0f;
		m_sToDOFBuf.Axyz[i] = 0.0f;
	}

	int t_timing = 0;
	while (dof_neutral != m_sReturnedDataFromDOF.nDOFStatus)
	{
		SendTo(&m_sToDOFBuf, sizeof(m_sToDOFBuf), m_nControllerPort, m_tcaControllerIP);
		Sleep(10);
		t_timing++;
		if (t_timing >= m_MaxDelay)
		{
			return -1;
			//AfxMessageBox(TEXT("DOF_ToMedian failed!\r\nPlease check the manual!"));
			//do something
			//exit(-1);
		}
	} 

	return 0;
}


int CConnectToController::DOF_ToBottom()
{
	m_sToDOFBuf.nCmd = S_CMD_Stop;
	for (int i = 0; i<6; i++)
	{
		m_sToDOFBuf.DOFs[i] = 0.0f;
	}
	for (int i = 0; i<3; i++)
	{
		m_sToDOFBuf.Vxyz[i] = 0.0f;
		m_sToDOFBuf.Axyz[i] = 0.0f;
	}

	int t_timing = 0;
	do
	{
		SendTo(&m_sToDOFBuf, sizeof(m_sToDOFBuf), m_nControllerPort, m_tcaControllerIP);
		Sleep(10);
		t_timing++;
		if (t_timing >= (m_MaxDelay/10/100))
		{
			return -1;
			//AfxMessageBox(TEXT("DOF_ToBottom failed!\r\nPlease check the manual!"));
			//do something
			//exit(-1);
		}
	} while (dof_check_id != m_sReturnedDataFromDOF.nDOFStatus);

	return 0;
}

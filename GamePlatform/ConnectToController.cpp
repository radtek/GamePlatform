#include "stdafx.h"
#include "ConnectToController.h"


CConnectToController::CConnectToController()
{
	TCHAR tControllerIP[17] = TEXT("192.168.0.125");
	_tcscpy_s(m_tcaControllerIP,tControllerIP);
	m_nControllerPort = 5000;
	memset(&m_sToDOFBuf,0,sizeof(m_sToDOFBuf));
	memset(&m_sToDOFBuf2, 0, sizeof(m_sToDOFBuf2));
	memset(&m_sReturnedDataFromDOF, 0, sizeof(m_sReturnedDataFromDOF));
	memset(m_tcaControllerIP, 0, sizeof(TCHAR)*17);
	m_sToDOFBuf.nCheckID = 55;
	m_sToDOFBuf2.nCheckID = 55;
}


CConnectToController::~CConnectToController()
{
}

void CConnectToController::OnReceive(int nErrorCode)
{
	int t_nRet=0;
	DataToHost t_sReturnedDataFromDOF;
	t_nRet = ReceiveFrom(&t_sReturnedDataFromDOF, sizeof(t_sReturnedDataFromDOF), CString(m_tcaControllerIP), m_nControllerPort, 0);
	if (SOCKET_ERROR == t_nRet)
	{
		ErrorWarnOfReceiveFrom(GetLastError());
	}
	else if (sizeof(DataToHost) == t_nRet)
	{
		memcpy(&m_sReturnedDataFromDOF, &t_sReturnedDataFromDOF, sizeof(DataToHost));
	}
	else
	{
		//lost part data
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
			AfxMessageBox(TEXT("DOF_ToMedian failed!\r\nPlease check the manual!"));
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
		if (t_timing >= m_MaxDelay)
		{
			AfxMessageBox(TEXT("DOF_ToBottom failed!\r\nPlease check the manual!"));
			//do something
			//exit(-1);
		}
	} while (dof_check_id != m_sReturnedDataFromDOF.nDOFStatus);

	return 0;
}

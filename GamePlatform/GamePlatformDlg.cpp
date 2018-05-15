
// GamePlatformDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GamePlatform.h"
#include "GamePlatformDlg.h"

#include "SimConnect.h"
#include "mmsystem.h"						//Timer
#include "resource.h"
#include "Resource.h"

#define		MAX_ANGULAR_VELOCITY			(0.06f)
#define		MAX_ANGULAR_ACC					(0.0f)
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#pragma comment(lib,"SimConnectDebug.lib") /*SimConnectDebug.lib*/
#else
#pragma comment(lib,"SimConnect.lib") 
#endif


void CALLBACK TimeProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
UINT __cdecl ThreadForSimConnect(LPVOID pParam);
void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);
UINT __cdecl ThreadPrepareProcess(LPVOID pParam);
void OnReceiveForExpansion(LPVOID pParam, int nErrorCode);
void OnReceiveForExternalDevice(LPVOID pParam, int nErrorCode);
void OnReceiveForSimtools(LPVOID pParam, int nErrorCode);
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGamePlatformDlg dialog



CGamePlatformDlg::CGamePlatformDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGamePlatformDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_uiMMTimer = 0;
	m_bProgramEndEnable = 0;

	memset(&m_sConfigParameterList, 0, sizeof(m_sConfigParameterList));
	m_bPitchReverseFlag = FALSE;
	m_bRollReverseFlag = FALSE;
	m_bYawReverseFlag = FALSE;

	m_bGameStartedFlag=FALSE;

	m_fprePitchDegrees=0.0f;
	m_fprePrePitchDegrees = 0.0f;
	m_fpreBankDegrees = 0.0f;
	m_fpreHeadDegrees = 0.0f;

	memset(&m_sPitchParaList, 0, sizeof(MotionParaList));
	m_sPitchParaList.fMaxAngularVel = MAX_ANGULAR_VELOCITY;
	m_sPitchParaList.fMaxAngularAcc = MAX_ANGULAR_ACC;

	memset(&m_sRollParaList, 0, sizeof(MotionParaList));
	m_sRollParaList.fMaxAngularVel = MAX_ANGULAR_VELOCITY;
	m_sRollParaList.fMaxAngularAcc = MAX_ANGULAR_ACC;

	m_bStepSignalFlag=FALSE;

	m_nCosFunctionTiming=0;
	m_nTimeFactor=1;
	m_fStepSignalDiffrence=0.0f;
	m_fValueBeforStepSignal=0.0f;

	memset(&m_sP3D_Para, 0, sizeof(m_sP3D_Para));

	m_pThreadForSimConnect = nullptr;

	m_bThreadEnableForSimConnect = TRUE;
	m_hSimConnect = NULL;
	for (int i = 0; i < 3; i++)
	{
		m_faPHB_Buffer[i] = 0.0f;
	}

	m_iQuit = 0;

	for (int i = 0; i < 8; i++)
	{
		m_faKnots[i] = i*5.0f-15.0f;
	}

	for (int i = 0; i < 4; i++)
	{
		m_faPitchCoefs[i] = 0.0f;
	}
	for (int i = 0; i < 4; i++)
	{
		m_faRollCoefs[i] = 0.0f;
	}
	for (int i = 0; i < 4; i++)
	{
		m_faYawCoefs[i] = 0.0f;
	}
	m_FnvalTiming = 0;

	//psAttitude = nullptr;
	memset(&m_fPreHead_w,0,sizeof(float)*5);
	m_bSimConnectSuccessFlag=FALSE;

	memset(&m_sAircraftPanel, 0, sizeof(FSX_Panel));

	m_csRemoteIP =_T("192.168.1.110");
	m_nRemotePort=10005;
}

void CGamePlatformDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGamePlatformDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CGamePlatformDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_GAME_CONFIG, &CGamePlatformDlg::OnBnClickedGameConfig)
	ON_MESSAGE(WM_USER_TRAYICON_NOTIFY, &CGamePlatformDlg::OnMouseOnTrayicon)
	ON_COMMAND(IDRCANCEL, &CGamePlatformDlg::OnRcancel)
	ON_WM_QUERYENDSESSION()
	ON_COMMAND(ID_SHOW_DLG, &CGamePlatformDlg::OnShowDlg)
	ON_COMMAND(ID_TO_MIDDLE, &CGamePlatformDlg::OnToMiddle)
	ON_COMMAND(ID_TO_BOTTOM, &CGamePlatformDlg::OnToBottom)
	ON_WM_ENDSESSION()
END_MESSAGE_MAP()



// CGamePlatformDlg message handlers

BOOL CGamePlatformDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	ShowWindow(SW_HIDE);
	// TODO: Add extra initialization here

	m_hSingleProMutex = CreateMutex(NULL, TRUE, _T("GamePlatform"));
	if (m_hSingleProMutex)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			AfxMessageBox(_T("ALREADY_EXISTS!\r\n"));
			exit(0);
		}
	}
	else
	{
		exit(0);
	}

	//NOTIFYICON
	_tcscpy_s(m_szTip,_T("MOTUS"));
	m_hTrayMenu = LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_TRAYICON));
	NotifyIconInit(AfxGetMainWnd()->m_hWnd, IDI_ICON1, WM_USER_TRAYICON_NOTIFY, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRAYICON)), m_szTip);
	NotifyIconShow();

	GetNecessaryDataFromConfigFile(NameOfConfigFlie);

	CheckProcessMutex(m_sConfigParameterList.tcaGameName);
	//Controller
	_tcscpy_s(ConnectToController.m_tcaControllerIP, m_sConfigParameterList.tcaControllerIP);
	ConnectToController.m_nControllerPort = m_sConfigParameterList.nControllerPort;
	ConnectToController.AsyncSocketInit(m_sConfigParameterList.nPortForController, SOCK_DGRAM,63L,m_sConfigParameterList.tcaLocalIP);

	//External Device
	m_CConnectToExternalDevice.UserOnReceive = OnReceiveForExternalDevice;
	m_CConnectToExternalDevice.UserObject = this;
	m_CConnectToExternalDevice.AsyncSocketInit(10002, SOCK_DGRAM, 63L, _T("192.168.1.101"));

	if (0 == _tcscmp(m_sConfigParameterList.tcaGameName, TEXT("P3D")))
	{
		m_ConnectToExpansion.UserOnReceive = OnReceiveForExpansion;
		m_ConnectToExpansion.UserObject = this;
		m_ConnectToExpansion.AsyncSocketInit(m_sConfigParameterList.nPortForExpansion, SOCK_DGRAM, 63L, m_sConfigParameterList.cLocalIPforExpansion);
	}
	else if (0 == _tcscmp(m_sConfigParameterList.tcaGameName, TEXT("DIRT3")))
	{
		//Simtools
		m_CConnectToLocalSoft.UserOnReceive = OnReceiveForSimtools;
		m_CConnectToLocalSoft.UserObject = this;
		m_CConnectToLocalSoft.AsyncSocketInit(5123, SOCK_DGRAM, 63L, _T("192.168.0.131"));
	}
	else
	{
		//nothing
	}
	

	//Sleep(2000);
	//CWinThread *pclThreadForExpansion = AfxBeginThread(ThreadForExpansion, this);
	Sleep(2000);//wait mcu data;
	CWinThread *t_thread = AfxBeginThread(ThreadPrepareProcess, this);

	m_uiMMTimer = ::timeSetEvent(10, 0, TimeProc, (DWORD_PTR)this, TIME_PERIODIC);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGamePlatformDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGamePlatformDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGamePlatformDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGamePlatformDlg::GetNecessaryDataFromConfigFile(LPCTSTR lpFileName)
{
	SpecialFunctions.GetStringFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("GAME_NAME"), NULL, m_sConfigParameterList.tcaGameName, sizeof(TCHAR)* 64, lpFileName);
	if (NULL == m_sConfigParameterList.tcaGameName)
	{
		AfxMessageBox(TEXT("NO Game is selected!\r\nPlease Check!"));
		exit(-1);
	}

	SpecialFunctions.GetStringFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("GAME_FOLDER_PATH"), NULL, m_sConfigParameterList.tcaGameFolderPath, sizeof(TCHAR)* 128, lpFileName);
	if (NULL == m_sConfigParameterList.tcaGameFolderPath)
	{
		AfxMessageBox(TEXT("GameFolderPath is empty!\r\nPlease Check!"));
		exit(-1);
	}

	SpecialFunctions.GetStringFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("GAME_EXE_FILE_PATH"), NULL, m_sConfigParameterList.tcaGameExeFilePath, sizeof(TCHAR)* 128, lpFileName);
	if (NULL == m_sConfigParameterList.tcaGameExeFilePath)
	{
		AfxMessageBox(TEXT("GameExeFilePath is empty!\r\nPlease Check!"));
		exit(-1);
	}
	
	TCHAR tLocalIP[17] = {0};
	SpecialFunctions.GetStringFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("LOCAL_IP"), NULL, tLocalIP, sizeof(TCHAR)* 17, lpFileName);
	if ((NULL != tLocalIP))
	{
		_tcscpy_s(m_sConfigParameterList.tcaLocalIP, tLocalIP);
	}
	else
	{
		SpecialFunctions.WriteStringToConfigFile(TEXT("GAME_PARAMETER"), TEXT("LOCAL_IP"), TEXT("192.168.0.131"), lpFileName);
	}
	
	m_sConfigParameterList.nPortForController = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("PORT_FOR_CONTROLLER"), 10000, lpFileName);


	m_sConfigParameterList.nPortForSoftware = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("PORT_FOR_ATTITUDE_SOURCE"), 5123, lpFileName);


	TCHAR tControllerIP[17] = {0};
	SpecialFunctions.GetStringFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("CONTROLLER_IP"), NULL, tControllerIP, sizeof(TCHAR)* 17, lpFileName);
	if ((NULL != tControllerIP))
	{
		_tcscpy_s(m_sConfigParameterList.tcaControllerIP, tControllerIP);
	}
	else
	{
		_tcscpy_s(m_sConfigParameterList.tcaControllerIP, TEXT("192.168.0.125"));
		SpecialFunctions.WriteStringToConfigFile(TEXT("GAME_PARAMETER"), TEXT("CONTROLLER_IP"), m_sConfigParameterList.tcaControllerIP, lpFileName);
	}

	m_sConfigParameterList.nControllerPort = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("CONTROLLER_PORT"), 5000, lpFileName);

	TCHAR tLocalIPforExpansion[17] = { 0 };
	SpecialFunctions.GetStringFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("LOCAL_IP_FOR_EXPANSION"), NULL, tLocalIPforExpansion, sizeof(TCHAR) * 17, lpFileName);
	if ((NULL != tLocalIPforExpansion))
	{
		_tcscpy_s(m_sConfigParameterList.cLocalIPforExpansion, tLocalIPforExpansion);
	}
	else
	{
		SpecialFunctions.WriteStringToConfigFile(TEXT("GAME_PARAMETER"), TEXT("LOCAL_IP_FOR_EXPANSION"), TEXT("192.168.0.132"), lpFileName);
	}

	m_sConfigParameterList.nPortForExpansion = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("PORT_FOR_EXPANSION"), 10001, lpFileName);


	TCHAR tExpansionIP[17] = { 0 };
	SpecialFunctions.GetStringFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("EXPANSION_IP"), NULL, tExpansionIP, sizeof(TCHAR) * 17, lpFileName);
	if ((NULL != tExpansionIP))
	{
		_tcscpy_s(m_sConfigParameterList.tcaExpansionIP, tExpansionIP);
	}
	else
	{
		_tcscpy_s(m_sConfigParameterList.tcaExpansionIP, TEXT("192.168.0.126"));
		SpecialFunctions.WriteStringToConfigFile(TEXT("GAME_PARAMETER"), TEXT("EXPANSION_IP"), m_sConfigParameterList.tcaExpansionIP, lpFileName);
	}

	m_sConfigParameterList.nExpansionPort = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("EXPANSION_PORT"), 5000, lpFileName);

	m_sConfigParameterList.eWorkMode = (PlatformWorkMode)SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("WORK_MODE"), 0, lpFileName);
	if ((FREE_MODE != m_sConfigParameterList.eWorkMode) && (QR_SCAN_MODE != m_sConfigParameterList.eWorkMode) && (IC_SCAN_MODE != m_sConfigParameterList.eWorkMode))
	{
		AfxMessageBox(TEXT("WorkMode is Error!\r\nPlease Check!"));
		exit(-1);
	}

	m_sConfigParameterList.bExternalControlEnable = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("EXTERNAL_CONTROL_ENABLE"), 0, lpFileName);


	m_sConfigParameterList.fK_Yaw = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K_YAW"), 1, lpFileName)/100.0f;
	m_sConfigParameterList.fK_Pitch = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K_PITCH"), 1, lpFileName)/100.0f;
	m_sConfigParameterList.fK_Roll = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K_ROLL"), 1, lpFileName)/100.0f;
	m_sConfigParameterList.fK_Surge = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K_SURGE"), 1, lpFileName)/100.0f;
	m_sConfigParameterList.fK_Sway = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K_SWAY"), 1, lpFileName)/100.0f;
	m_sConfigParameterList.fK_Heave = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K_HEAVE"), 1, lpFileName)/100.0f;

	//Additional
	m_sConfigParameterList.fK1_Surge = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K1_SURGE"), 1, lpFileName) / 100.0f;
	m_sConfigParameterList.fK1_Sway = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("K1_SWAY"), 1, lpFileName) / 100.0f;

	m_sConfigParameterList.bDlgEnable = SpecialFunctions.GetIntDataFromConfigFile(TEXT("GAME_PARAMETER"), TEXT("DLG_ENABLE"), 0, lpFileName);
	

}

void CGamePlatformDlg::CheckProcessMutex(LPCTSTR lpName)
{
	HANDLE m_hSingleProMutex;

	m_hSingleProMutex = CreateMutex(NULL, TRUE, lpName);
	if (NULL == m_hSingleProMutex)
	{
		AfxMessageBox(TEXT("CreateMutex failed!\r\n"));
		exit(-1);
	}
	else
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			AfxMessageBox(TEXT("ProcessMutex already existed!\r\n"));
			exit(-1);
		}
		else
		{
			//create ProcessMutex successfully
		}
	}
}


int CGamePlatformDlg::GamesCheckAndPrepare(LPCTSTR lpName)
{
	if (0 == _tcscmp(lpName, TEXT("P3D")))
	{
		if (NULL == ::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")))
		{
			HINSTANCE ret = ShellExecute(0, TEXT("open"), m_sConfigParameterList.tcaGameExeFilePath, TEXT(""), TEXT(""), SW_SHOWMINIMIZED);
			if (ret <= (HINSTANCE)32)
			{
				AfxMessageBox(TEXT("Game fail to open!\r\nPlease Check!"));
				exit(-1);
			}
			Sleep(3000);
		}
		int t_timing = 0;
		while (NULL == ::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")))
		{
			Sleep(1000);
			t_timing++;
			if (15 <= t_timing)
			{
				AfxMessageBox(TEXT("Game fail to run!\r\nPlease Check!"));
				exit(-1);
			}
		}

		Sleep(3000);


		HRESULT hr;
		if (SUCCEEDED(SimConnect_Open(&m_hSimConnect, "Prepar3D", NULL, 0, 0, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL)))
		{
			//printf("\nConnected to Prepar3D!");

			//// Set up the data definition, but do not yet do anything with it
			// Set up a data definition for the throttle control
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_THROTTLE, \
				"GENERAL ENG PROPELLER LEVER POSITION:1", "percent");					//控制直升机杆上的throttle
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_THROTTLE, \
				"GENERAL ENG THROTTLE LEVER POSITION:1", "percent");
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_THROTTLE, \
				"GENERAL ENG THROTTLE LEVER POSITION:2", "percent");
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_THROTTLE, \
				"GENERAL ENG THROTTLE LEVER POSITION:3", "percent");
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_THROTTLE, \
				"GENERAL ENG THROTTLE LEVER POSITION:4", "percent");

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_ELEVATOR, "ELEVATOR POSITION", "Position");   //Control pitch
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_RUDDER, "RUDDER POSITION", "Position");   //Control yaw
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_AILERON, "AILERON POSITION", "Position");   //Control roll

			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_OTHER, "GENERAL ENG THROTTLE LEVER POSITION:1", "percent");
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_OTHER, "GENERAL ENG MIXTURE LEVER POSITION:1", "percent");
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_OTHER, "GENERAL ENG PROPELLER LEVER POSITION:1", "percent");	//控制直升机杆上的throttle
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_OTHER, "ROTOR LATERAL TRIM PCT", "percent");
			//hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_OTHER, "COLLECTIVE POSITION", "percentage");		

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_BRAKE, "BRAKE LEFT POSITION", "Position");
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_BRAKE, "BRAKE RIGHT POSITION", "Position");
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_BRAKE, "BRAKE PARKING POSITION", "Bool");

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_ACC, "ACCELERATION WORLD X", "Feet per second squared");
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_ACC, "ACCELERATION WORLD Y", "Feet per second squared");
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_ACC, "ACCELERATION WORLD Z", "Feet per second squared");
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_ACC, "ACCELERATION BODY X", "Feet per second squared");
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_ACC, "ACCELERATION BODY Y", "Feet per second squared");
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_ACC, "ACCELERATION BODY Z", "Feet per second squared");
			//Panel
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "VERTICAL SPEED", "Feet per second", SIMCONNECT_DATATYPE_FLOAT32);

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "PLANE PITCH DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "PLANE BANK DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT32);

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "PLANE HEADING DEGREES MAGNETIC", "degrees", SIMCONNECT_DATATYPE_FLOAT32);

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "AIRSPEED INDICATED", "Knots", SIMCONNECT_DATATYPE_FLOAT32);

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "GENERAL ENG RPM:1", "Percent", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "ENG ROTOR RPM:1", "Percent", SIMCONNECT_DATATYPE_INT32);

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "INDICATED ALTITUDE", "Feet", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "MAGNETIC COMPASS", "Degrees", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "RECIP ENG MANIFOLD PRESSURE:1", "inHg"/*"Psi"*/, SIMCONNECT_DATATYPE_FLOAT32);

			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "ELECTRICAL GENALT BUS AMPS:1", "Amperes", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "GENERAL ENG OIL PRESSURE:1", "PSI"/*"Psf"*/, SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "FUEL TANK RIGHT AUX QUANTITY", "Gallons", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "ENG OIL TEMPERATURE:1", "fahrenheit"/*"Rankine"*/, SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "FUEL TANK LEFT MAIN QUANTITY", "Gallons", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "RECIP ENG CYLINDER HEAD TEMPERATURE:1", "fahrenheit"/*"Celsius"*/, SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "RECIP CARBURETORTEMPERATURE:1", "Celsius", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "AMBIENT TEMPERATURE", "Celsius", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_PANEL, "COLLECTIVE POSITION", "Percent", SIMCONNECT_DATATYPE_INT32);
			//lights
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "ROTOR CLUTCH ACTIVE", "Bool", SIMCONNECT_DATATYPE_INT32);
			//ignore M.R. GearBox Temp light
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "ROTOR CHIP DETECTED", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "ROTOR BRAKE ACTIVE", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "GENERAL ENG STARTER ACTIVE", "Bool", SIMCONNECT_DATATYPE_INT32);
			//ignore T.R.GearBox Chip Light,尾旋翼齿轮箱金属屑灯
			//ignore LOW FUEL LIGHT,低油量灯
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "ROTOR GOV ACTIVE", "Bool", SIMCONNECT_DATATYPE_INT32);

			//ignore LOW RPM LIGHT,低转速灯
			//ignore ALT LOW VOLTAGE LIGHT,低电压灯
			//ignore OIL PRESSURE LIGHT,滑油压力灯	
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "ROTOR GOV ACTIVE", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "LIGHT NAV", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "STROBES AVAILABLE", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "GENERAL ENG GENERATOR ACTIVE:1", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "ELECTRICAL MASTER BATTERY", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "MASTER IGNITION SWITCH", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "RECIP ENG LEFT MAGNETO:1", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "RECIP ENG RIGHT MAGNETO:1", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "TURB ENG IGNITION SWITCH", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "CARB HEAT AVAILABLE", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "ROTOR LATERAL TRIM PCT", "percent", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "LIGHT LANDING", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "RECIP MIXTURE RATIO:1", "Ratio", SIMCONNECT_DATATYPE_FLOAT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "GENERAL ENG FUEL VALVE:1", "Bool", SIMCONNECT_DATATYPE_INT32);
			hr = SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_LIGHTS_SWITCHS, "LIGHT PANEL", "Bool", SIMCONNECT_DATATYPE_INT32);
			//
			
			//// Request an event when the simulation starts
			hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, EVENT_SIM_START, "SimStart");

			hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, EVENT_FLIGHT_LOAD, "FlightLoaded");
			hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, EVENT_FLIGHT_CRASHED, "Crashed");
			//hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, EVENT_RECUR_1SEC, "1sec");
			Sleep(10);
			m_pThreadForSimConnect = AfxBeginThread(ThreadForSimConnect, this);
			SimConnect_CallDispatch(m_hSimConnect, ::MyDispatchProcRD, this);
			Sleep(3000);
			m_bSimConnectSuccessFlag = TRUE;
		}
		else
		{
			AfxMessageBox(TEXT("SimConnect_Open Failed!\r\nPlease Check!"));
			exit(-1);
		}

		
	}
	else if (0 == _tcscmp(lpName, TEXT("DIRT3")))
	{	
		
		ShellExecute(0, _T("open"), _T("C:\\Program Files (x86)\\SimTools\\SimTools_GameManager.EXE"), _T(""), _T(""), SW_SHOWMINIMIZED);
		Sleep(5000);
		ShellExecute(0, _T("open"), _T("C:\\Program Files (x86)\\SimTools\\SimTools_GameEngine.EXE"), _T(""), _T(""), SW_SHOWMINIMIZED);
		Sleep(5000);
		/*if (NULL == ::FindWindow(NULL, TEXT("DIRT 3")))
		{
			HINSTANCE ret = ShellExecute(0, TEXT("open"), m_sConfigParameterList.tcaGameExeFilePath, TEXT(""), m_sConfigParameterList.tcaGameFolderPath, SW_SHOWMINIMIZED);
			if (ret <= (HINSTANCE)32)
			{
				AfxMessageBox(TEXT("Game fail to open!\r\nPlease Check!"));
				exit(-1);
			}
		}
		int t_timing = 0;
		while (NULL == ::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")))
		{
			Sleep(1000);
			t_timing++;
			if (15 <= t_timing)
			{
				AfxMessageBox(TEXT("Game fail to run!\r\nPlease Check!"));
				exit(-1);
			}
		}*/
		Sleep(3000);
	}
	else
	{
		AfxMessageBox(TEXT("Maybe GameName Wrong!\r\nPlease Check!"));
		exit(-1);
	}
	return 0;
}
int CGamePlatformDlg::P3D_ExternalControlDataProcess(DataToHost tsDataToHost)
{
	double tdValue;
	if ((85 < tsDataToHost.attitude[0]) && (210 > tsDataToHost.attitude[0]))
	{
		tdValue = (tsDataToHost.attitude[0]-85) / 1.25;
		if (100.0 <= tdValue)
		{
			tdValue = 100.0;
		}
		else if (0.0 >= tdValue)
		{
			tdValue = 0.0;
		}
	}
	else if ((85 >= tsDataToHost.attitude[0]) && (0 <= tsDataToHost.attitude[0]))
	{
		tdValue = 0;
	}
	else if ((210 <= tsDataToHost.attitude[0]) && (255 >= tsDataToHost.attitude[0]))
	{
		tdValue = 100;
	}
	else
	{
		tdValue = m_sP3D_Para.sThrottles.dThrottle1;
	}
	m_sP3D_Para.sThrottles.dThrottle1 = tdValue;

	if ((50 < tsDataToHost.nRev1) && (210 > tsDataToHost.nRev1))
	{
		tdValue = (210-tsDataToHost.nRev1)/1.6;
	}
	else if ((50 >= tsDataToHost.nRev1) && (0 <= tsDataToHost.nRev1))
	{
		tdValue = 100;
	}
	else if ((210 <= tsDataToHost.nRev1) && (255 >= tsDataToHost.nRev1))
	{
		tdValue = 0;
	}
	else
	{
		tdValue=m_sP3D_Para.sOtherControl.nCollectivePosition;
	}
	m_sP3D_Para.sOtherControl.nCollectivePosition = tdValue;
	//m_sP3D_Para.sThrottles.dThrottle1 = 2000;

	//m_sP3D_Para.sThrottles.dThrottle2 = m_sP3D_Para.sThrottles.dThrottle1;
	//pitch
	//m_sP3D_Para.sElevator.dElevatorPercent = (ConnectToController.m_sReturnedDataFromDOF.motor_code[1] - 4095.0 / 2.0) / (4095.0 / 2.0);
	//if (1.0 <= m_sP3D_Para.sElevator.dElevatorPercent)
	//{
	//	m_sP3D_Para.sElevator.dElevatorPercent = 1.0;
	//}
	//else if (-1.0 >= m_sP3D_Para.sElevator.dElevatorPercent)
	//{
	//	m_sP3D_Para.sElevator.dElevatorPercent = -1.0;
	//}
	//else if ((-0.05 <= m_sP3D_Para.sElevator.dElevatorPercent) && (0.05 >= m_sP3D_Para.sElevator.dElevatorPercent))
	//{
	//	m_sP3D_Para.sElevator.dElevatorPercent = 0.0;
	//}
	////roll
	//m_sP3D_Para.sAileron.dAileronPercent = (ConnectToController.m_sReturnedDataFromDOF.motor_code[2] - 4095.0 / 2.0) / (4095.0 / 2.0);
	//if (1.0 <= m_sP3D_Para.sAileron.dAileronPercent)
	//{
	//	m_sP3D_Para.sAileron.dAileronPercent = 1.0;
	//}
	//else if (-1.0 >= m_sP3D_Para.sAileron.dAileronPercent)
	//{
	//	m_sP3D_Para.sAileron.dAileronPercent = -1.0;
	//}
	//else if ((-0.05 <= m_sP3D_Para.sAileron.dAileronPercent) && (0.05 >= m_sP3D_Para.sAileron.dAileronPercent))
	//{
	//	m_sP3D_Para.sAileron.dAileronPercent = 0.0;
	//}
	//m_sP3D_Para.sAileron.dAileronPercent = 0;
	//ruder
	if ((80 < tsDataToHost.nRev0) && (100 > tsDataToHost.nRev0))
	{
		tdValue = 0;
	}
	else if ((80 > tsDataToHost.nRev0) && (0 <= tsDataToHost.nRev0))
	{
		tdValue = (80.0-tsDataToHost.nRev0) / 75.0;
		if (1.0<tdValue)
		{
			tdValue = 1.0;
		}
	}
	else if ((100 < tsDataToHost.nRev0) && (255 >= tsDataToHost.nRev0))
	{
		tdValue = (100.0-tsDataToHost.nRev0) / 70.0;
		
		if (-1.0>tdValue)
		{
			tdValue = -1.0;
		}
	}
	else
	{
		tdValue = m_sP3D_Para.sRudder.dRudderPercent;
	}
	m_sP3D_Para.sRudder.dRudderPercent = tdValue;

	//m_sP3D_Para.sRudder.dRudderPercent = 0;
	//brake
	/*m_sP3D_Para.sBrake.dBrakePercentL = ConnectToController.m_sReturnedDataFromDOF.motor_code[4] / 40.95;
	if (5.0 > m_sP3D_Para.sBrake.dBrakePercentL)
	{
		m_sP3D_Para.sBrake.dBrakePercentL = 0.0;
	}
	else if (100 <= m_sP3D_Para.sBrake.dBrakePercentL)
	{
		m_sP3D_Para.sBrake.dBrakePercentL = 100.0;
	}

	m_sP3D_Para.sBrake.dBrakePercentR = m_sP3D_Para.sBrake.dBrakePercentL;*/
	return 0;
}
int CGamePlatformDlg::P3D_DataProcess()
{
	if (FALSE == m_bStepSignalFlag)
	{
		ConnectToController.m_sToDOFBuf.nCheckID = 55;
		ConnectToController.m_sToDOFBuf.nCmd = 0;

		ConnectToController.m_sToDOFBuf.DOFs[0] = SpecialFunctions.fnval(m_faKnots, m_faPitchCoefs, m_FnvalTiming)*0.4;
		ConnectToController.m_sToDOFBuf.DOFs[1] = SpecialFunctions.fnval(m_faKnots, m_faRollCoefs, m_FnvalTiming)*0.4;
		ConnectToController.m_sToDOFBuf.DOFs[2] = (SpecialFunctions.fnval(m_faKnots, m_faYawCoefs, m_FnvalTiming))*0.2f; //0.0f;
		ConnectToController.m_sToDOFBuf.DOFs[3] = 0.0f;
		ConnectToController.m_sToDOFBuf.DOFs[4] = 0.0f;
		ConnectToController.m_sToDOFBuf.DOFs[5] = 0.0f;

		m_sPitchParaList.fDesPos = ConnectToController.m_sToDOFBuf.DOFs[0];
		m_sPitchParaList.fMaxDesPos = 15.0;
		ConnectToController.m_sToDOFBuf.DOFs[0] = SpecialFunctions.LimitVelAndACC(&m_sPitchParaList);
		m_sRollParaList.fDesPos = ConnectToController.m_sToDOFBuf.DOFs[1];
		m_sRollParaList.fMaxDesPos = 15.0;
		ConnectToController.m_sToDOFBuf.DOFs[1] = SpecialFunctions.LimitVelAndACC(&m_sRollParaList);

		ConnectToController.SendTo(&(ConnectToController.m_sToDOFBuf), sizeof(ConnectToController.m_sToDOFBuf), m_sConfigParameterList.nControllerPort, m_sConfigParameterList.tcaControllerIP);

		/*SimConnect_SetDataOnSimObject(m_hSimConnect, DEFINITION_THROTTLE, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(P3D_ThrottleControl), &m_sP3D_Para.sThrottles);
		SimConnect_SetDataOnSimObject(m_hSimConnect, DEFINITION_ELEVATOR, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(P3D_ElevatorControl), &m_sP3D_Para.sElevator);
		SimConnect_SetDataOnSimObject(m_hSimConnect, DEFINITION_AILERON, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(P3D_AileronControl), &m_sP3D_Para.sAileron);
		SimConnect_SetDataOnSimObject(m_hSimConnect, DEFINITION_BRAKE, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(P3D_BrakeControl), &m_sP3D_Para.sBrake);*/
		if (m_FnvalTiming >= 4)
		{
			//SimConnect_RequestDataOnSimObjectType(m_hSimConnect, REQUEST_ATTITUDE, DEFINITION_ATTITUDE, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
			SimConnect_RequestDataOnSimObjectType(m_hSimConnect, REQUEST_PANEL, DEFINITION_PANEL, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
			SimConnect_RequestDataOnSimObjectType(m_hSimConnect, REQUEST_LIGHTS_SWITCHS, DEFINITION_LIGHTS_SWITCHS, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
			SimConnect_CallDispatch(m_hSimConnect, ::MyDispatchProcRD, this);



			m_faPitchCoefs[0] = m_faPitchCoefs[1];
			m_faPitchCoefs[1] = m_faPitchCoefs[2];
			m_faPitchCoefs[2] = m_faPitchCoefs[3];
			m_faPitchCoefs[3] = m_faPHB_Buffer[0];

			m_faRollCoefs[0] = m_faRollCoefs[1];
			m_faRollCoefs[1] = m_faRollCoefs[2];
			m_faRollCoefs[2] = m_faRollCoefs[3];
			m_faRollCoefs[3] = m_faPHB_Buffer[1];

			m_faYawCoefs[0] = m_faYawCoefs[1];
			m_faYawCoefs[1] = m_faYawCoefs[2];
			m_faYawCoefs[2] = m_faYawCoefs[3];
			m_faYawCoefs[3] = m_faPHB_Buffer[2];

			m_FnvalTiming = 0;
		}
		else
		{
			m_FnvalTiming++;
		}
	}
	
	return 0;
}
int CGamePlatformDlg::DIRT3_DataProcess()
{
	if (0xEEEE == m_sSimtoolsData.Head)
	{
		ConnectToController.DOF_ToMedian();
	}
	else if (0xFFFF == m_sSimtoolsData.Head)
	{
		ConnectToController.m_sToDOFBuf.nCheckID = 55;
		ConnectToController.m_sToDOFBuf.nCmd = 0;

		ConnectToController.m_sToDOFBuf.DOFs[3] = (static_cast<float>(static_cast<INT16>(m_sSimtoolsData.Sway - 0x7FFF)) / 10000 / 1000)	* m_sConfigParameterList.fK_Sway*100.0f;
		ConnectToController.m_sToDOFBuf.DOFs[4] = (static_cast<float>(static_cast<INT16>(m_sSimtoolsData.Surge - 0x7FFF)) / 10000 / 1000)	* m_sConfigParameterList.fK_Surge*100.0f;
		ConnectToController.m_sToDOFBuf.DOFs[5] = (static_cast<float>(static_cast<INT16>(m_sSimtoolsData.Heave - 0x7FFF)) / 10000 / 1000)	* m_sConfigParameterList.fK_Heave*10.0f*100.0f;
		ConnectToController.m_sToDOFBuf.DOFs[1] = (static_cast<float>(static_cast<INT16>(m_sSimtoolsData.Roll - 0x7FFF)) / 10000)			* m_sConfigParameterList.fK_Roll*100.0f \
			+ ConnectToController.m_sToDOFBuf.DOFs[3] * m_sConfigParameterList.fK1_Sway*100.0f;
		ConnectToController.m_sToDOFBuf.DOFs[0] = (static_cast<float>(static_cast<INT16>(m_sSimtoolsData.Pitch - 0x7FFF)) / 10000)			* m_sConfigParameterList.fK_Pitch*100.0f \
			+ ConnectToController.m_sToDOFBuf.DOFs[4] * m_sConfigParameterList.fK1_Surge*100.0f;
		ConnectToController.m_sToDOFBuf.DOFs[2] = 0;//(static_cast<float>(static_cast<INT16>(m_sSimtoolsData.Yaw - 0x7FFF)) / 10000)			* m_sConfigParameterList.nK_Yaw;
		

		ConnectToController.SendTo(&(ConnectToController.m_sToDOFBuf), sizeof(ConnectToController.m_sToDOFBuf), m_sConfigParameterList.nControllerPort, m_sConfigParameterList.tcaControllerIP);
	}
	else if (0xBBBB == m_sSimtoolsData.Head)
	{
	}
	char t_buffer[128];
	sprintf_s(t_buffer, sizeof(t_buffer), "%.3f#%.3f#%.3f#%.3f#%.3f#%.3f#%.3f#%.3f#", m_sConfigParameterList.fK_Pitch, m_sConfigParameterList.fK_Roll, \
		m_sConfigParameterList.fK_Yaw, m_sConfigParameterList.fK_Surge, m_sConfigParameterList.fK_Sway, \
		m_sConfigParameterList.fK_Heave, m_sConfigParameterList.fK1_Surge, m_sConfigParameterList.fK1_Sway);
	m_CConnectToExternalDevice.SendTo(t_buffer, sizeof(t_buffer), m_nRemotePort, m_csRemoteIP);
	return 0;
}
void OnReceiveForExpansion(LPVOID pParam, int nErrorCode)
{
	int t_nRet = 0;
	DataToHost tsReturnedDataFromExpansion;
	CGamePlatformDlg *pGamePlatformDlg = (CGamePlatformDlg *)pParam;
	if (true == pGamePlatformDlg->m_bSimConnectSuccessFlag)
	{
		t_nRet = pGamePlatformDlg->m_ConnectToExpansion.ReceiveFrom(&tsReturnedDataFromExpansion, sizeof(tsReturnedDataFromExpansion), CString(pGamePlatformDlg->m_sConfigParameterList.tcaExpansionIP), pGamePlatformDlg->m_sConfigParameterList.nExpansionPort, 0);
		if (SOCKET_ERROR == t_nRet)
		{
			pGamePlatformDlg->m_ConnectToExpansion.ErrorWarnOfReceiveFrom(GetLastError());
		}
		else if (sizeof(DataToHost) == t_nRet)
		{
			memcpy(&(pGamePlatformDlg->m_sReturnedDataFromExpansion), &tsReturnedDataFromExpansion, sizeof(DataToHost));
			pGamePlatformDlg->P3D_ExternalControlDataProcess(pGamePlatformDlg->m_sReturnedDataFromExpansion);
		}
		else
		{
			//lost part data
		}
		
		//pGamePlatformDlg->m_sDataToExpansion.uiLightsFlag = 0xF0F0F0F0;			//TEST LIGHTS
		if (4 == pGamePlatformDlg->m_FnvalTiming)
		{
#ifdef USE_EXTERNAL_CONTROL
			if (2< (pGamePlatformDlg->m_sP3D_Para.sOtherControl.nCollectivePosition - pGamePlatformDlg->m_sAircraftPanel.nCollectivePosition))				//Destination-Current
			{
				PostMessage(::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")), WM_SYSKEYDOWN, VK_F3, 0);
				PostMessage(::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")), WM_SYSKEYUP, VK_F3, 0);
			}
			else if (-2 > (pGamePlatformDlg->m_sP3D_Para.sOtherControl.nCollectivePosition - pGamePlatformDlg->m_sAircraftPanel.nCollectivePosition))		//Destination-Current
			{
				PostMessage(::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")), WM_SYSKEYDOWN, VK_F2, 0);
				PostMessage(::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")), WM_SYSKEYUP, VK_F2, 0);
			}

			SimConnect_SetDataOnSimObject(pGamePlatformDlg->m_hSimConnect, DEFINITION_THROTTLE, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(P3D_ThrottleControl), &pGamePlatformDlg->m_sP3D_Para.sThrottles);
			SimConnect_SetDataOnSimObject(pGamePlatformDlg->m_hSimConnect, DEFINITION_RUDDER, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(P3D_RudderControl), &pGamePlatformDlg->m_sP3D_Para.sRudder);
#endif
		}

		t_nRet = pGamePlatformDlg->m_ConnectToExpansion.SendTo(&(pGamePlatformDlg->m_sDataToExpansion), sizeof(pGamePlatformDlg->m_sDataToExpansion), pGamePlatformDlg->m_sConfigParameterList.nExpansionPort, pGamePlatformDlg->m_sConfigParameterList.tcaExpansionIP);

		if (SOCKET_ERROR == t_nRet)
		{
			pGamePlatformDlg->m_ConnectToExpansion.ErrorWarnOfSendTo(GetLastError());
		}
		else
		{
			//lost part data
		}
	}
}

void OnReceiveForExternalDevice(LPVOID pParam, int nErrorCode)
{
	TCHAR t_tcReceiveData[128];
	int t_nRet = 0;
	ConfigParameterList t_sConfigParameterList;
	CGamePlatformDlg *pGamePlatformDlg = (CGamePlatformDlg *)pParam;
	t_nRet = pGamePlatformDlg->m_CConnectToExternalDevice.ReceiveFrom(&t_tcReceiveData, sizeof(t_tcReceiveData), \
		pGamePlatformDlg->m_csRemoteIP, pGamePlatformDlg->m_nRemotePort, 0);
	if (SOCKET_ERROR == t_nRet)
	{
		//pGamePlatformDlg->m_CConnectToExternalDevice.ErrorWarnOfReceiveFrom(GetLastError());
	}
	else
	{
		if (8 == sscanf_s((const char *)t_tcReceiveData, "%f#%f#%f#%f#%f#%f#%f#%f#", &t_sConfigParameterList.fK_Pitch, &t_sConfigParameterList.fK_Roll, \
			&t_sConfigParameterList.fK_Yaw, &t_sConfigParameterList.fK_Surge, &t_sConfigParameterList.fK_Sway, \
			&t_sConfigParameterList.fK_Heave, &t_sConfigParameterList.fK1_Surge, &t_sConfigParameterList.fK1_Sway))
		{
			memcpy(&pGamePlatformDlg->m_sConfigParameterList.fK_Pitch, &t_sConfigParameterList, sizeof(float)*8);
		}
	}
}

void OnReceiveForSimtools(LPVOID pParam, int nErrorCode)
{
	TCHAR t_tcReceiveData[128];
	int t_nRet = 0;
	CString t_ip;
	UINT t_port;
	ConfigParameterList t_sConfigParameterList;
	CGamePlatformDlg *pGamePlatformDlg = (CGamePlatformDlg *)pParam;
	s_simtools_chardata dh;
	int i = 0;

	if (sizeof(s_simtools_chardata) == pGamePlatformDlg->m_CConnectToLocalSoft.ReceiveFrom(&dh, sizeof(s_simtools_chardata),t_ip,t_port))
	{
		//recvfrom(Socket_udp_connect_with_simtools, (char *)&dh, sizeof(s_simtools_chardata), 0, (SOCKADDR *)&simtools_Addr, &simtools_AddrSize);
		for (i = 0; i < 4; i++)
		{
			pGamePlatformDlg->SpecialFunctions.CharToHex(&dh.simtools_data_head_char[i]);
			pGamePlatformDlg->SpecialFunctions.CharToHex(&dh.simtools_data_pitch_char[i]);
			pGamePlatformDlg->SpecialFunctions.CharToHex(&dh.simtools_data_roll_char[i]);
			pGamePlatformDlg->SpecialFunctions.CharToHex(&dh.simtools_data_sway_char[i]);
			pGamePlatformDlg->SpecialFunctions.CharToHex(&dh.simtools_data_surge_char[i]);
			pGamePlatformDlg->SpecialFunctions.CharToHex(&dh.simtools_data_heave_char[i]);
			pGamePlatformDlg->SpecialFunctions.CharToHex(&dh.simtools_data_yaw_char[i]);
		}

		pGamePlatformDlg->m_sSimtoolsData.Head = ((dh.simtools_data_head_char[0]) << 12) | ((dh.simtools_data_head_char[1]) << 8) | ((dh.simtools_data_head_char[2]) << 4) | ((dh.simtools_data_head_char[3]) << 0);
		pGamePlatformDlg->m_sSimtoolsData.Pitch = ((dh.simtools_data_pitch_char[0]) << 12) | ((dh.simtools_data_pitch_char[1]) << 8) | ((dh.simtools_data_pitch_char[2]) << 4) | ((dh.simtools_data_pitch_char[3]) << 0);
		pGamePlatformDlg->m_sSimtoolsData.Roll = ((dh.simtools_data_roll_char[0]) << 12) | ((dh.simtools_data_roll_char[1]) << 8) | ((dh.simtools_data_roll_char[2]) << 4) | ((dh.simtools_data_roll_char[3]) << 0);
		pGamePlatformDlg->m_sSimtoolsData.Sway = ((dh.simtools_data_sway_char[0]) << 12) | ((dh.simtools_data_sway_char[1]) << 8) | ((dh.simtools_data_sway_char[2]) << 4) | ((dh.simtools_data_sway_char[3]) << 0);
		pGamePlatformDlg->m_sSimtoolsData.Surge = ((dh.simtools_data_surge_char[0]) << 12) | ((dh.simtools_data_surge_char[1]) << 8) | ((dh.simtools_data_surge_char[2]) << 4) | ((dh.simtools_data_surge_char[3]) << 0);
		pGamePlatformDlg->m_sSimtoolsData.Heave = ((dh.simtools_data_heave_char[0]) << 12) | ((dh.simtools_data_heave_char[1]) << 8) | ((dh.simtools_data_heave_char[2]) << 4) | ((dh.simtools_data_heave_char[3]) << 0);
		pGamePlatformDlg->m_sSimtoolsData.Yaw = ((dh.simtools_data_yaw_char[0]) << 12) | ((dh.simtools_data_yaw_char[1]) << 8) | ((dh.simtools_data_yaw_char[2]) << 4) | ((dh.simtools_data_yaw_char[3]) << 0);
	}
}

UINT __cdecl ThreadForSimConnect(LPVOID pParam)
{
	CGamePlatformDlg *pGamePlatformDlg = (CGamePlatformDlg *)pParam;
	while (TRUE == pGamePlatformDlg->m_bThreadEnableForSimConnect)
	{
		if (NULL != ::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3")))
		{
			if (TRUE == pGamePlatformDlg->m_sConfigParameterList.bExternalControlEnable)
			{
				//pGamePlatformDlg->P3D_ExternalControlDataProcess();
				
			}
			
			//SimConnect_CallDispatch(pGamePlatformDlg->m_hSimConnect, MyDispatchProcRD, pGamePlatformDlg);
		}
		
	}
	return 0;
}

UINT __cdecl ThreadPrepareProcess(LPVOID pParam)
{
	CGamePlatformDlg *pGamePlatformDlg = (CGamePlatformDlg *)pParam;
#ifdef USE_DOF
	pGamePlatformDlg->ConnectToController.DOF_UpToMedian();
	pGamePlatformDlg->ConnectToController.DOF_ToMedian();
	pGamePlatformDlg->ConnectToController.DOF_ToRun();
#endif
	pGamePlatformDlg->GamesCheckAndPrepare(pGamePlatformDlg->m_sConfigParameterList.tcaGameName);
	pGamePlatformDlg->m_bGameStartedFlag = TRUE;
	return 0;
}



void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
	HRESULT hr;
	DWORD ObjectID1;
	//DWORD ObjectID2;
	//DWORD ObjectID3;
	CHAR t_debugmessage[128];

	
	CGamePlatformDlg *pGamePlatformDlg = (CGamePlatformDlg *)pContext;

 	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_OPEN:
	{
		//hr = SimConnect_RequestDataOnSimObjectType(pGamePlatformDlg->m_hSimConnect, REQUEST_ATTITUDE, DEFINITION_ATTITUDE, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
		hr = SimConnect_RequestDataOnSimObjectType(pGamePlatformDlg->m_hSimConnect, REQUEST_PANEL, DEFINITION_PANEL, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
		hr = SimConnect_RequestDataOnSimObjectType(pGamePlatformDlg->m_hSimConnect, REQUEST_LIGHTS_SWITCHS, DEFINITION_LIGHTS_SWITCHS, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
		pGamePlatformDlg->m_bSimConnectSuccessFlag = TRUE;
		break;
	}
	case SIMCONNECT_RECV_ID_EVENT:
	{
		SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;
		switch (evt->uEventID)
		{
		case EVENT_SIM_START:
			OutputDebugString(TEXT("EVENT_SIM_START!\r\n"));
			break;
		case EVENT_FLIGHT_CRASHED:
			OutputDebugString(TEXT("Flight Crashed!\r\n"));
			break;
		default:
			break;
		}
		break;
	}
	case SIMCONNECT_RECV_ID_EVENT_FILENAME:
	{
		SIMCONNECT_RECV_EVENT_FILENAME *evt = (SIMCONNECT_RECV_EVENT_FILENAME*)pData;
		switch (evt->uEventID)
		{
		case EVENT_FLIGHT_LOAD:
			OutputDebugString(TEXT("Flight Load!\r\n"));
			break;
		default:
			break;
		}
		break;
	}
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
	{
		SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

		switch (pObjData->dwRequestID)
		{
		case REQUEST_PANEL:
		{
			ObjectID1 = pObjData->dwObjectID;
			memcpy(&pGamePlatformDlg->m_sAircraftPanel, &pObjData->dwData, sizeof(FSX_Panel));
			//pGamePlatformDlg->m_psAircraftPanel = (FSX_Panel *)&pObjData->dwData;

			int tAngleToPulseValue=0;								//临时记录用角度值

			CString strOutput;

			pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed = pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed * 0.6;
			tAngleToPulseValue = static_cast<int>((-0.0001441*pow(pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed, 5)) + 3.872e-05*pow(pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed, 4) \
				+ 0.08375*pow(pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed, 3) + (-0.007153*pow(pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed, 2)) \
				+ 4.929*pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed + 353.8);
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (700 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 700;
			}
			pGamePlatformDlg->m_sDataToExpansion.nVerticalSpeed = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fVerticalSpeed);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_VERTICAL_SPEED, strOutput);
#endif
			if (((pGamePlatformDlg->m_sAircraftPanel.sAttitude.BankDegrees) <= 0.000001f)\
				&& ((pGamePlatformDlg->m_sAircraftPanel.sAttitude.PitchDegrees) <= 0.000001f)\
				&& (fabsf(pGamePlatformDlg->m_sAircraftPanel.sAttitude.HeadingDegreesMagnetic) <= 0.000001f))
			{

			}
			else
			{
				pGamePlatformDlg->m_faPHB_Buffer[0] = pGamePlatformDlg->m_sAircraftPanel.sAttitude.PitchDegrees;
				pGamePlatformDlg->m_faPHB_Buffer[1] = pGamePlatformDlg->m_sAircraftPanel.sAttitude.BankDegrees;
				pGamePlatformDlg->m_faPHB_Buffer[2] = 0;
			}
			//PITCH
			tAngleToPulseValue = static_cast<int>(12.35 * pGamePlatformDlg->m_sAircraftPanel.sAttitude.PitchDegrees - 420);
			if (-100 < tAngleToPulseValue)
			{
				tAngleToPulseValue = -100;
			}
			else if (-450 > tAngleToPulseValue)
			{
				tAngleToPulseValue = -450;
			}
			pGamePlatformDlg->m_sDataToExpansion.nPitch = tAngleToPulseValue;
			//空速
			tAngleToPulseValue = static_cast<int>(6 * pGamePlatformDlg->m_sAircraftPanel.fAirSpeed + (-120));
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (600 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 600;
			}

			pGamePlatformDlg->m_sDataToExpansion.nAirSpeed = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fAirSpeed);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_AIRSPEED_INDICATED, strOutput);
#endif
			//转速
			tAngleToPulseValue = static_cast<int>(-3.302*pGamePlatformDlg->m_sAircraftPanel.uiRotorRPM + 130.4);
			if (0 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (-260 > tAngleToPulseValue)
			{
				tAngleToPulseValue = -260;
			}
			pGamePlatformDlg->m_sDataToExpansion.nEngineRPM = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%d"), pGamePlatformDlg->m_sAircraftPanel.uiEngineRPM);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_GENERAL_ENG_RPM, strOutput);
#endif
			//转速
			tAngleToPulseValue = static_cast<int>(3.333*pGamePlatformDlg->m_sAircraftPanel.uiRotorRPM - 100);
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (290 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 290;
			}

			pGamePlatformDlg->m_sDataToExpansion.nRotorRPM = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%d"), pGamePlatformDlg->m_sAircraftPanel.nCollectivePosition/*uiRotorRPM*/);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ENG_ROTOR_RPM, strOutput);
#endif
			//高度   as roll
			tAngleToPulseValue = static_cast<int>(10.72*pGamePlatformDlg->m_sAircraftPanel.sAttitude.BankDegrees + 560);
			/*if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (1525 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 1525;
			}*/
			//还需要做其他运算处理
			pGamePlatformDlg->m_sDataToExpansion.nAltimeter = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fAltimeter);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ALTITUDE, strOutput);
#endif
			//罗盘
			tAngleToPulseValue = static_cast<int>(-2 * pGamePlatformDlg->m_sAircraftPanel.fCompass + 360);

			pGamePlatformDlg->m_sDataToExpansion.nCompass = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fCompass);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_MAGNETIC_COMPASS, strOutput);
#endif
			//进气压力表
			tAngleToPulseValue = static_cast<int>(14.31*pGamePlatformDlg->m_sAircraftPanel.fManifoldPressure + (-28.81));
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (400 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 400;
			}

			pGamePlatformDlg->m_sDataToExpansion.nManifoldPressure = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fManifoldPressure);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_RECIP_ENG_MANIFOLD_PRESSURE, strOutput);
#endif
			//电流表
			tAngleToPulseValue = static_cast<int>(0.7188*pGamePlatformDlg->m_sAircraftPanel.fAmmeter + 79.08);
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (130 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 130;
			}
			pGamePlatformDlg->m_sDataToExpansion.nAmmeter = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fAmmeter);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ELECTRICAL_GENALT_BUS_AMPS, strOutput);
#endif
			//机油压力表
			tAngleToPulseValue = static_cast<int>(2.5*pGamePlatformDlg->m_sAircraftPanel.fOilPressure + 312.5)*2*1.5;
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (1400 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 1400;
			}

			pGamePlatformDlg->m_sDataToExpansion.nOilPressure = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fOilPressure);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_GENERAL_ENG_OIL_PRESSURE, strOutput);
#endif

			//副油箱
			tAngleToPulseValue = static_cast<int>(1.2*pGamePlatformDlg->m_sAircraftPanel.fMainFuelQuantity + (-1.953e-14))*5;
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (120 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 120;
			}

			pGamePlatformDlg->m_sDataToExpansion.nAuxFuelQuantity = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fAuxFuelQuantity);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_FUEL_TANK_LEFT_AUX_QUANTITY, strOutput);
#endif

			//机油温度表
			tAngleToPulseValue = static_cast<int>(1.294*pGamePlatformDlg->m_sAircraftPanel.fOilTemperature + (-67.06));
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (245 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 245;
			}

			pGamePlatformDlg->m_sDataToExpansion.nOilTemperature = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fOilTemperature);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ENG_OIL_TEMPERATURE, strOutput);
#endif

			//主油箱
			tAngleToPulseValue = static_cast<int>(1.2*pGamePlatformDlg->m_sAircraftPanel.fMainFuelQuantity + (-1.953e-14))*5;
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (120 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 120;
			}

			pGamePlatformDlg->m_sDataToExpansion.nMainFuelQuantity = tAngleToPulseValue;
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fMainFuelQuantity);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_FUEL_TANK_LEFT_MAIN_QUANTITY, strOutput);
#endif

			//
			tAngleToPulseValue = static_cast<int>(0.4*pGamePlatformDlg->m_sAircraftPanel.fCylinderHeadTemperature + (-80))*5;
			if (0 > tAngleToPulseValue)
			{
				tAngleToPulseValue = 0;
			}
			else if (500 < tAngleToPulseValue)
			{
				tAngleToPulseValue = 500;
			}
			pGamePlatformDlg->m_sDataToExpansion.nCylinderHeadTemperature = tAngleToPulseValue;//static_cast<int>(pGamePlatformDlg->m_sAircraftPanel.fCylinderHeadTemperature * 90 / 19);
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fCylinderHeadTemperature);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_RECIP_ENG_CYLINDER_HEAD_TEMPERATURE, strOutput);
#endif
			//CarburetorTemperature 先用于高度，因为之前暂定不用高度表，因此将高度值用于roll的值传送给下位机
			tAngleToPulseValue = static_cast<int>(pGamePlatformDlg->m_sAircraftPanel.fAltimeter/10*62);
			pGamePlatformDlg->m_sDataToExpansion.nCarburetorTemperature = tAngleToPulseValue;
			//pGamePlatformDlg->m_sDataToExpansion.nCarburetorTemperature = static_cast<int>(pGamePlatformDlg->m_sAircraftPanel.fCarburetorTemperature+45);//温度单位的转换
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fCarburetorTemperature);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_RECIP_CARBURETOR_TEMPERATURE, strOutput);
#endif
			pGamePlatformDlg->m_sDataToExpansion.nAirTemperature = static_cast<int>(pGamePlatformDlg->m_sAircraftPanel.fAirTemperature);
#ifdef SHOW_DLG
			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sAircraftPanel.fAirTemperature);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_AMBIENT_TEMPERATURE, strOutput);
#endif
		}
			break;
		case REQUEST_LIGHTS_SWITCHS:
		{
			ObjectID1 = pObjData->dwObjectID;
			memcpy(&pGamePlatformDlg->m_sLightsAndSwitchs, &pObjData->dwData, sizeof(FSX_LightsAndSwitchs));
			CString strOutput;

			if (90 >= pGamePlatformDlg->m_sAircraftPanel.uiRotorRPM)
			{
				pGamePlatformDlg->m_sLightsAndSwitchs.nLowRPM = 1;
			}
			else
			{
				pGamePlatformDlg->m_sLightsAndSwitchs.nLowRPM = 0;
			}

			if ((pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nEngineRightMagneto != 1) && \
				(pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nEngineLeftMagneto != 1))
			{
				pGamePlatformDlg->m_sLightsAndSwitchs.nStartOn = 1;
			}
			else
			{
				pGamePlatformDlg->m_sLightsAndSwitchs.nStartOn = 0;
			}

			pGamePlatformDlg->m_sDataToExpansion.uiLightsFlag = (pGamePlatformDlg->m_sLightsAndSwitchs.nPanelLights << 17) | (pGamePlatformDlg->m_sLightsAndSwitchs.nFuelValveSwitch << 16) | \
				(pGamePlatformDlg->m_sLightsAndSwitchs.nLandingLightsSwitch << 15) | (pGamePlatformDlg->m_sLightsAndSwitchs.nCarburetorHeatSwitch << 14) | \
				(pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nTurbEngineIgnitionSwitch << 13) | \
				(pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nEngineRightMagneto << 12) | \
				(pGamePlatformDlg->m_sLightsAndSwitchs.nNAV_LTS << 11) | \
				/*(pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nRotorGoverner << 10) |*/ \
				/*(pGamePlatformDlg->m_sLightsAndSwitchs.nLowOilPressure << 9) | *//*(pGamePlatformDlg->m_sLightsAndSwitchs.nAlternatorLight << 8) |*/ \
				(pGamePlatformDlg->m_sLightsAndSwitchs.nLowRPM << 7) |/* (pGamePlatformDlg->m_sLightsAndSwitchs.nLowFuel << 6) |*/ \
				/*((pGamePlatformDlg->m_sLightsAndSwitchs.nTR_Chip) << 5) | */(pGamePlatformDlg->m_sLightsAndSwitchs.nStartOn << 4) | \
				(pGamePlatformDlg->m_sLightsAndSwitchs.nRotorBrake<<3)|/*(pGamePlatformDlg->m_sLightsAndSwitchs.nMR_Chip << 2) |*/ \
				/*(pGamePlatformDlg->m_sLightsAndSwitchs.nMR_Temp << 1) | */(pGamePlatformDlg->m_sLightsAndSwitchs.nRotorClutch << 0);
#ifdef SHOW_DLG
			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nRotorClutch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ROTOR_CLUTCH_STATE, strOutput);

			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sLightsAndSwitchs.nMR_Chip);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ROTOR_CHIP_LIGHT, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nRotorBrake);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ROTOR_BRAKE_SWITCH, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nStartOn);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_GENERAL_ENG_STARTER_ACTIVE, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nRotorGoverner);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ROTOR_GOV_ACTIVE, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nNAV_LTS);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_LIGHT_NAV, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nStrobe);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_STROBES_AVAILABLE, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nAlternatorSwitch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_GENERAL_ENG_GENERATOR_ACTIVE, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nMasterBatterySwitch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ELECTRICAL_MASTER_BATTERY, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nMasterIgnitionSwitch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_MASTER_IGNITION_SWITCH, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nEngineLeftMagneto);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_RECIP_ENG_LEFT_MAGNETO, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nEngineRightMagneto);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_RECIP_ENG_RIGHT_MAGNETO, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.sIgnitionAndMagnetorsSwitch.nTurbEngineIgnitionSwitch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_TURB_ENG_IGNITION_SWITCH, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nCarburetorHeatSwitch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_CARB_HEAT_AVAILABLE, strOutput);

			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sLightsAndSwitchs.fRotorTrim);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_ROTOR_LATERAL_TRIM_PCT, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nLandingLightsSwitch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_LIGHT_LANDING, strOutput);

			strOutput.Format(_T("%4f"), pGamePlatformDlg->m_sLightsAndSwitchs.fMixtureControl);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_RECIP_MIXTURE_RATIO, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nFuelValveSwitch);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_GENERAL_ENG_FUEL_VALVE, strOutput);

			strOutput.Format(_T("%4d"), pGamePlatformDlg->m_sLightsAndSwitchs.nPanelLights);
			pGamePlatformDlg->SetDlgItemText(IDC_SHOW_LIGHT_PANEL, strOutput);
#endif
		}
			break;
		default:
			break;
		}
		break;
	}


	case SIMCONNECT_RECV_ID_QUIT:
	{
		pGamePlatformDlg->m_iQuit = 1;
		break;
	}

	default:
		//printf("\nReceived:%d",pData->dwID);
		break;
	}
}

void CALLBACK TimeProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	
	CGamePlatformDlg *pGamePlatformDlg = (CGamePlatformDlg *)dwUser;
	if (TRUE == pGamePlatformDlg->m_bGameStartedFlag)
	{
		if ((0 == _tcscmp(pGamePlatformDlg->m_sConfigParameterList.tcaGameName, TEXT("P3D"))) && (NULL != ::FindWindow(NULL, TEXT("Lockheed Martin® Prepar3D® v3"))) && (TRUE == pGamePlatformDlg->m_bSimConnectSuccessFlag))
		{
			pGamePlatformDlg->P3D_DataProcess();
		}
		else /*if (0 == _tcscmp(pGamePlatformDlg->m_sConfigParameterList.tcaGameName, TEXT("DIRT3")) && (NULL != ::FindWindow(NULL, TEXT("DIRT 3"))))*/
		{
			pGamePlatformDlg->DIRT3_DataProcess();
		}
	}
	else
	{
		//nothing
	}
	
	
}



void CGamePlatformDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	GamesCheckAndPrepare(m_sConfigParameterList.tcaGameName);

	//CDialogEx::OnOK();
}


void CGamePlatformDlg::OnBnClickedGameConfig()
{
	// TODO: Add your control notification handler code here

}

BOOL CGamePlatformDlg::NotifyIconInit(HWND hWnd,UINT uID,UINT nNotifyMsg,HICON hIcon,LPCTSTR lpTip)
{
	m_bVisibled = FALSE;
	m_uiTrayNotifyMsg = nNotifyMsg;
	//init NOTIFYICONDATA
	memset(&m_nidata, 0, sizeof(m_nidata));
	m_nidata.cbSize = sizeof(m_nidata);
	m_nidata.uID = uID;
	m_nidata.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	m_nidata.uCallbackMessage = m_uiTrayNotifyMsg;
	m_nidata.hIcon = hIcon;
	if (lpTip)
	{
		_tcsncpy_s(m_nidata.szTip, lpTip,sizeof(m_nidata.szTip)/sizeof(TCHAR)-1);
	}
	else
	{
		m_nidata.szTip[0] = _T('\0');
	}
	m_nidata.hWnd = hWnd;
	return 0;
}
BOOL CGamePlatformDlg::NotifyIconShow()
{
	if (!m_bVisibled)
	{
		m_bVisibled = Shell_NotifyIcon(NIM_ADD, &m_nidata);

	}
	return m_bVisibled;
}
BOOL CGamePlatformDlg::NotifyIconHide()
{
	if (m_bVisibled)
	{
		m_bVisibled = (!Shell_NotifyIcon(NIM_DELETE, &m_nidata));

	}
	return(!m_bVisibled);
}
BOOL CGamePlatformDlg::NotifyIconSet(HICON hNewIcon)
{
	m_nidata.hIcon = hNewIcon;
	if (m_bVisibled)
	{
		BOOL bRet;
		bRet = Shell_NotifyIcon(NIM_MODIFY,&m_nidata);
		return bRet;
	}
	else
	{
		return TRUE;
	}
}

BOOL CGamePlatformDlg::NotifyIconSetTipText(LPCTSTR lpNewTipText)
{
	memset(m_nidata.szTip, 0, sizeof(m_nidata.szTip));
	_tcsncpy_s(m_nidata.szTip, lpNewTipText, sizeof(m_nidata.szTip) / sizeof(TCHAR)-1);
	if (m_bVisibled)
	{
		BOOL bRet;
		bRet = Shell_NotifyIcon(NIM_MODIFY, &m_nidata);
		return bRet;
	}
	else
		return TRUE;
}

BOOL CGamePlatformDlg::NotifyIconIsVisibled()
{
	return m_bVisibled;
}

BOOL CGamePlatformDlg::NotifyIconRebuild()
{
	if (m_bVisibled)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_nidata);
		m_bVisibled = FALSE;
		return NotifyIconShow();
	}
	else
	{
		return FALSE;
	}
}

afx_msg LRESULT CGamePlatformDlg::OnMouseOnTrayicon(WPARAM wParam, LPARAM lParam)
{
	if (IDI_TRAYICON == wParam)
	{
		switch (lParam)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetForegroundWindow();
			POINT pt = { 0 };
			GetCursorPos(&pt);
			TrackPopupMenu(GetSubMenu(m_hTrayMenu, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y,0,this->m_hWnd, NULL);
			break;
		}
	}
	return 0;
}


void CGamePlatformDlg::OnRcancel()
{
	// TODO:  Add your specialized query end session code here
	m_bGameStartedFlag = FALSE;
	ConnectToController.DOF_ToMedian();
	exit(0);
}


BOOL CGamePlatformDlg::OnQueryEndSession()
{
	if (!CDialogEx::OnQueryEndSession())
		return FALSE;
	// TODO:  Add your specialized query end session code here
	m_bGameStartedFlag = FALSE;
	//ConnectToController.DOF_ToBottom();
	return TRUE;
}


void CGamePlatformDlg::OnShowDlg()
{
	// TODO:  Add your specialized query end session code here
	ShowWindow(SW_SHOW);
}


void CGamePlatformDlg::OnToMiddle()
{
	// TODO:  Add your specialized query end session code here
	if (dof_check_id == ConnectToController.m_sReturnedDataFromDOF.nDOFStatus)
	{
		ConnectToController.DOF_UpToMedian();
		ConnectToController.DOF_ToMedian();
		m_bGameStartedFlag = TRUE;
	}
	else if (dof_working == ConnectToController.m_sReturnedDataFromDOF.nDOFStatus)
	{
		m_bGameStartedFlag = FALSE;
		ConnectToController.DOF_ToMedian();
	}
	else if (dof_neutral == ConnectToController.m_sReturnedDataFromDOF.nDOFStatus)
	{
		m_bGameStartedFlag = TRUE;
	}
}


void CGamePlatformDlg::OnToBottom()
{
	// TODO:  Add your specialized query end session code here
	m_bGameStartedFlag = FALSE;
	//ConnectToController.DOF_ToBottom();
}


void CGamePlatformDlg::OnEndSession(BOOL bEnding)
{
	CDialogEx::OnEndSession(bEnding);
	// TODO:  Add your specialized query end session code here
	m_bGameStartedFlag = FALSE;
	//ConnectToController.DOF_ToBottom();
	exit(0);
}

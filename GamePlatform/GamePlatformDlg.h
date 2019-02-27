
// GamePlatformDlg.h : header file
//

#pragma once

#include "ConnectToController.h"										//connect to master control board
#include "ConnectToAttitudeSource.h"										//connect to simtools software/P3D game software/other
#include "SpecialFunctions.h"
#include "SimConnect.h"
#include "SharedMemory.h"

#define WM_USER_TRAYICON_NOTIFY	(WM_USER+102)
//P3D
struct P3D_Attitude
{
	float PitchDegrees;// PLANE PITCH DEGREES 
	float BankDegrees;// PLANE BANK DEGREES 
	//float HeadingDegrees_true;// PLANE HEADING DEGREES TRUE      not need;
	float HeadingDegreesMagnetic;//PLANE HEADING DEGREES MAGNETIC 
};
struct P3D_Acc
{
	float AccWorldX;// ACCELERATION WORLD X;
	float AccWorldY;// ACCELERATION WORLD Y;
	float AccWorldZ;// ACCELERATION WORLD Z;

	float AccBodyX;//ACCELERATION BODY X 
	float AccBodyY;//ACCELERATION BODY Y
	float AccBodyZ;//ACCELERATION BODY Z 
};
static enum P3D_EventID{
	EVENT_SIM_START,
	EVENT_W,//PITCH
	EVENT_S,

	EVENT_A,//ROLL
	EVENT_D,

	EVENT_Q,//HEADING
	EVENT_E,

	EVENT_X,//THROTTLE
	EVENT_Z,

	EVENT_FLIGHT_LOAD,
	EVENT_FLIGHT_CRASHED,

	EVENT_RECUR_1SEC,
};
static enum P3D_DataDefineID {
	DEFINITION_THROTTLE,
	DEFINITION_BRAKE,

	DEFINITION_ELEVATOR,
	DEFINITION_RUDDER,
	DEFINITION_AILERON,

	//DEFINITION_ATTITUDE,					//PITCH ROLL(BANK) YAW(HEADING)    aircraft  DEGREES
	DEFINITION_ACC,							//aircraft  acc

	DEFINITION_PANEL,						//Panel data
	DEFINITION_LIGHTS_SWITCHS,

	DEFINITION_OTHER,
};


static enum P3D_DataRequestID {
	REQUEST_THROTTLE,
	REQUEST_BRAKE,

	REQUEST_ELEVATOR,
	REQUEST_RUDDER,
	REQUEST_AILERON,

	//REQUEST_ATTITUDE,					//PITCH ROLL(BANK) YAW(HEADING)//aircraft	rotation
	REQUEST_ACC,						//aircraft  acc

	REQUEST_PANEL,
	REQUEST_LIGHTS_SWITCHS,

	REQUEST_OTHER,
};
struct P3D_ThrottleControl
{
	double dThrottle1;														//Percent
	//double dThrottle2;
	//double dThrottle3;
	//double dThrottle4;
};
struct P3D_ElevatorControl
{
	double dElevatorPercent;
};

struct P3D_RudderControl
{
	double dRudderPercent;
};
struct P3D_AileronControl
{
	double dAileronPercent;
};
struct P3D_BrakeControl
{
	double dBrakePercentL;
	double dBrakePercentR;
	bool   bHandbrake_flag;
};

struct P3D_OtherControl
{
	//double dThrottleLeverPosition;								//GENERAL ENG THROTTLE LEVER POSITION : index	Percent of max throttle position	Percent	Y	Shared Cockpit(Index of 1 only).
	//double dMixtureLeverPosition;								//GENERAL ENG MIXTURE LEVER POSITION:index	Percent of max mixture lever position	Percent	Y	Shared Cockpit (Index of 1 only).
	//double dPropellerLevrPostion;								//GENERAL ENG PROPELLER LEVER POSITION : index	Percent of max prop lever position	Percent	Y	Shared Cockpit(Index of 1 only).
	//double dRotorLateralTrimPCT;								//ROTOR LATERAL TRIM PCT	Trim percent	Percent Over 100	Y	-
	//double dRotorRPM;											//ROTOR RPM PCT	Percent max rated rpm	Percent Over 100	Y	-
	INT		nCollectivePosition;							//COLLECTIVE POSITION
};
struct P3D_Parameter
{
	//HANDLE  hSimConnect = NULL;

	P3D_ThrottleControl		sThrottles;
	P3D_ElevatorControl		sElevator;
	P3D_RudderControl		sRudder;
	P3D_AileronControl		sAileron;
	P3D_BrakeControl		sBrake;
	P3D_OtherControl		sOtherControl;
};
///
struct IgnitionAndMagnetorsSwitch
{
	INT nMasterIgnitionSwitch;											//													MASTER IGNITION SWITCH						bool
	INT nEngineLeftMagneto;												//													RECIP ENG LEFT MAGNETO:index				bool	Y
	INT nEngineRightMagneto;											//													RECIP ENG RIGHT MAGNETO:index				bool	Y
	INT nTurbEngineIgnitionSwitch;										//													TURB ENG IGNITION SWITCH					bool
};
struct FSX_LightsAndSwitchs
{
	INT nRotorClutch;													//CLUTCH LIGHT AND SWITCH,离合器灯和开关			ROTOR CLUTCH ACTIVE							bool
	//INT nMR_Temp;														//M.R. GearBox Temp light,主旋翼齿轮箱温度灯		ROTOR TEMPERATURE
	INT nMR_Chip;														//M.R. GearBox Chip Light,主旋翼齿轮箱金属屑灯		ROTOR CHIP DETECTED							bool
	INT nRotorBrake;													//ROTOR BRAKE LIGHT ADN SWITCH,旋翼刹车灯及开关		ROTOR BRAKE ACTIVE							bool
	
	INT nStartOn;														//STARTER-ON LIGHT,启动灯							GENERAL ENG STARTER ACTIVE
	//INT nTR_Chip;														//T.R.GearBox Chip Light,尾旋翼齿轮箱金属屑灯
	//INT nLowFuel;														//LOW FUEL LIGHT,低油量灯							
	INT nLowRPM;														//LOW RPM LIGHT,低转速灯							低转速警告灯和喇叭在转速≤97%时显示。

	//INT nAlternatorLight;//INT nLowVoltage;							//ALT LOW VOLTAGE LIGHT,低电压灯					ELECTRICAL GENALT BUS VOLTAGE:index			Volts
																		//GENERAL ENG OIL PRESSURE : index			Psf
	//INT nLowOilPressure;												//OIL PRESSURE LIGHT,滑油压力灯						下限红线 25 psi（磅/平方英寸）
	INT nRotorGoverner;													//GOVERNOR-OFF LIGHT ADN SWITCH,调速器灯及开关		ROTOR GOV ACTIVE							bool

	INT nNAV_LTS;														//NAV lights and switch;							LIGHT NAV									bool
	INT nStrobe;														//STROBE LIGHT AND SWITCH,							STROBES AVAILABLE							bool
	//
	INT nAlternatorSwitch;												//													GENERAL ENG GENERATOR ACTIVE:index			bool	Y
	INT nMasterBatterySwitch;											//													ELECTRICAL MASTER BATTERY					bool	Y
	IgnitionAndMagnetorsSwitch	sIgnitionAndMagnetorsSwitch;			
	INT nCarburetorHeatSwitch;											//													CARB HEAT AVAILABLE							bool
	FLOAT	fRotorTrim;													//													ROTOR LATERAL TRIM PCT						percent
	INT	nLandingLightsSwitch;											//													LIGHT LANDING								bool
	FLOAT	fMixtureControl;											//													RECIP MIXTURE RATIO:index					Ratio	Y

	INT	nFuelValveSwitch;												//													GENERAL ENG FUEL VALVE:index				bool
	INT	nPanelLights;													//													LIGHT PANEL									bool
	//
};
//Aircraft Miscellaneous Data

//IS USER SIM										Is this the user loaded aircraft
//IS SLEW ACTIVE									True if slew is active
//IS SLEW ALLOWED									True if slew is enabled
//TYPICAL DESCENT RATE								Normal descent rate						Feet per minute
//CATEGORY											One of the following:
													//"Airplane",
													//"Helicopter",
													//"Boat",
													//"GroundVehicle",
													//"ControlTower",
													//"SimpleObject",
													//"Viewer"
//TOTAL VELOCITY									Velocity regardless of direction. For example, if a helicopter is ascending vertically at 100 fps, getting this variable will return 100.	Feet per second
//TURN INDICATOR RATE								Turn indicator reading	Radians per second


struct FSX_Panel
{
	float	fVerticalSpeed;												//VERT SPEED INDICATOR,升降速度表					VERTICAL SPEED								Feet per second
	P3D_Attitude sAttitude;
	float	fAirSpeed;													//AIRSPEED INDICATED,空速表							AIRSPEED INDICATED							Knots
	UINT	uiEngineRPM;												//ENGINE TACK,发动机转速表							GENERAL ENG RPM:index						Rpm
	UINT	uiRotorRPM;													//ROTOR TACK,旋翼转速表								ENG ROTOR RPM:index							Percent scalar 16K (Max rpm * 16384)
	
	float	fAltimeter;													//ALTIMETER,高度表									INDICATED ALTITUDE							Feet
	float	fCompass;													//													MAGNETIC COMPASS							Degrees
	float	fManifoldPressure;											//MANIFOLD PRESSURE,进气压力表						RECIP ENG MANIFOLD PRESSURE:index			Psi
	//ENGINE INSTRUMENTS,发动机仪表组合
	float	fAmmeter;													//AMMETER,电流表									ELECTRICAL GENALT BUS AMPS:index			Amperes
	float	fOilPressure;												//OIL PRESSURE										GENERAL ENG OIL PRESSURE : index			Psf
	float	fAuxFuelQuantity;											//													FUEL TANK LEFT AUX QUANTITY					Gallons
	float	fOilTemperature;											//													ENG OIL TEMPERATURE:index					Rankine
	float	fMainFuelQuantity;											//													FUEL TANK LEFT MAIN QUANTITY				Gallons
	float	fCylinderHeadTemperature;									//													RECIP ENG CYLINDER HEAD TEMPERATURE:index	Celsius

	float	fCarburetorTemperature;										//													RECIP CARBURETOR TEMPERATURE:index			Celsius
	float	fAirTemperature;											//													AMBIENT TEMPERATURE							Celsius
	int	nCollectivePosition;										//The position of the helicopter's collective. 0 is fully up, 100 fully depressed.
																		//													COLLECTIVE POSITION							Percent_over_100	Y
};

struct DataToExpansion
{
	BYTE nCheckID;
	BYTE nCmd;
	BYTE nAct;
	BYTE nReserved;									//保留

	int		nVerticalSpeed;
	//P3D_Attitude sAttitude;
	int		nAirSpeed;
	int		nEngineRPM;
	int		nRotorRPM;

	int		nAltimeter;
	int		nCompass;
	int		nManifoldPressure;
	//ENGINE INSTRUMENTS,发动机仪表组合
	int		nAmmeter;
	int		nOilPressure;
	int		nAuxFuelQuantity;
	int		nOilTemperature;
	int		nMainFuelQuantity;
	int		nCylinderHeadTemperature;

	int		nCarburetorTemperature;
	int		nAirTemperature;
	UINT	uiLightsFlag;
	int		nPitch;
	int		nUndefine2;
	int		nUndefine3;
	DataToExpansion()
	{
		nVerticalSpeed=355;
		//P3D_Attitude sAttitude;
		nAirSpeed=0;
		nEngineRPM=-30;
		nRotorRPM=60;

		nAltimeter=560;
		nCompass=0;
		nManifoldPressure=0;
		//ENGINE INSTRUMENTS,发动机仪表组合
		nAmmeter=0;
		nOilPressure=0;
		nAuxFuelQuantity=0;
		nOilTemperature=0;
		nMainFuelQuantity=0;
		nCylinderHeadTemperature=0;

		nCarburetorTemperature=0;
		nAirTemperature=0;
		uiLightsFlag=0xFFFFFFFF;
		nPitch = -420;
		nUndefine2=0;
		nUndefine3=0;
	}
};
struct ExpansionToHost
{
	UINT uiMixtureControl;
	UINT uiRotorTrim;
	UINT uiSwitchsFlag;
};

//DIRT3

struct s_simtools_chardata
{
	char simtools_data_head_char[4];
	char simtools_data_pitch_char[4];
	char simtools_data_roll_char[4];
	char simtools_data_sway_char[4];
	char simtools_data_surge_char[4];
	char simtools_data_heave_char[4];
	char simtools_data_yaw_char[4];
};

struct SimtoolsData
{
	UINT16 Head;
	UINT16 Pitch;
	UINT16 Roll;
	UINT16 Sway;
	UINT16 Surge;
	UINT16 Heave;
	UINT16 Yaw;
};
//COMMON STRUCT

enum PlatformWorkMode
{
	FREE_MODE,
	QR_SCAN_MODE,
	IC_SCAN_MODE
};
enum Pcar2RunStatus
{
	CLOSED=0,
	RUNNING=1
};
struct ConfigParameterList
{
	float fK_Pitch;
	float fK_Roll;
	float fK_Yaw;															//
	float fK_Surge;
	float fK_Sway;
	float fK_Heave;

	float fK1_Surge;											//additional
	float fK1_Sway;												//additional

	BOOL bDlgEnable;
	BOOL bExternalControlEnable;

	PlatformWorkMode eWorkMode;									//0: free;	1:QR code;	2:Scan IC

	TCHAR tcaGameName[64];											//
	TCHAR tcaGameFolderPath[128];									//
	TCHAR tcaGameExeFilePath[128];

	TCHAR tcaLocalIP[17];
	UINT nPortForSoftware;
	UINT nPortForController;

	TCHAR cLocalIPforExpansion[17];
	UINT nPortForExpansion;

	TCHAR cIpForMainControl[17];


	TCHAR tcaControllerIP[17];
	UINT nControllerPort;

	TCHAR tcaExpansionIP[17];
	UINT nExpansionPort;

	TCHAR tcExternalDeviceIP[17];
	UINT nExternalDevicePort;
};
// CGamePlatformDlg dialog
class CGamePlatformDlg : public CDialogEx
{
// Construction
public:
	CGamePlatformDlg(CWnd* pParent = NULL);	// standard constructor
	~CGamePlatformDlg();

// Dialog Data
	enum { IDD = IDD_GAMEPLATFORM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
//NOTIFYICON
public:
	HANDLE m_hSingleProMutex;

	NOTIFYICONDATA m_nidata;
	BOOL m_bVisibled;										//NOTIFYICON show or not;
	TCHAR m_szTip[64];
	HMENU m_hTrayMenu;

	UINT	m_uiTrayNotifyMsg;								//Message ID

	BOOL	NotifyIconInit(HWND hWnd, UINT uID, UINT nNotifyMsg, HICON hIcon = NULL, LPCTSTR lpTip = NULL);
	BOOL	NotifyIconShow();
	BOOL	NotifyIconHide();
	BOOL	NotifyIconSet(HICON hNewIcon);
	BOOL	NotifyIconSetTipText(LPCTSTR lpNewTipText);
	BOOL	NotifyIconIsVisibled();
	BOOL	NotifyIconRebuild();
public:
	UINT m_uiMMTimer;
	BOOL m_bProgramEndEnable;
	const float m_fPitchMax=18.0f;
	const float m_fPitchVelocityMax = 2.3f;
	const float m_fRollMax=8.0f;
	const float m_fRollVelocityMax = 1.0f;
	const float m_fYawMax = 10.0f;
	const float m_fYawVelocityMax = 0.5f;
public:
	const CString NameOfConfigFlie = { TEXT("Config1.ini") };
	HINSTANCE m_GameStartUpReturnValue;

	CConnectToController ConnectToController;							//connect to master control board
	CConnectToAttitudeSource ConnectToAttitudeSource;					//connect to simtools software/P3D game software/other
	CMT_AsyncSocket	m_ConnectToExpansion;								//Connect to second master control board for instrument panel
	CMT_AsyncSocket m_CConnectToExternalDevice;							//as IPAD,Phone
	CMT_AsyncSocket m_CConnectToLocalSoft;								//as Simtools Soft

	CString m_csRemoteIP;
	UINT m_nRemotePort;


	CSpecialFunctions SpecialFunctions;									//some functions

	ConfigParameterList m_sConfigParameterList;

	BOOL m_bPitchReverseFlag;
	BOOL m_bRollReverseFlag;
	BOOL m_bYawReverseFlag;

	const float m_fEffectivePitch=60.0f;
	const float m_fEffectiveRoll = 60.0f;
	const float m_fEffectiveYaw = 60.0f;

	BOOL m_bGameStartedFlag;

	float m_fprePitchDegrees;
	float m_fprePrePitchDegrees;
	float m_fpreBankDegrees;
	float m_fpreHeadDegrees;

	MotionParaList m_sPitchParaList;
	MotionParaList m_sRollParaList;


	BOOL m_bStepSignalFlag;
	UINT m_nCosFunctionTiming;
	UINT m_nTimeFactor;
	float m_fStepSignalDiffrence;
	float m_fValueBeforStepSignal;
public:
	void GetNecessaryDataFromConfigFile(LPCTSTR lpFileName);			//
	void CheckProcessMutex(LPCTSTR lpName);								//confirm only one process running
	int GamesCheckAndPrepare(LPCTSTR lpName);
	void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);
	//void ShowOrHideDlg
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedGameConfig();
	//User define
	afx_msg LRESULT OnMouseOnTrayicon(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()


public:
	
public:																	//P3D use parameter.
	FSX_Panel				m_sAircraftPanel;
	FSX_LightsAndSwitchs	m_sLightsAndSwitchs;
	//FSX_Panel				*m_psAircraftPanel;
	P3D_Parameter			m_sP3D_Para;
	CWinThread				*m_pThreadForSimConnect;
	BOOL					m_bThreadEnableForSimConnect;
	HANDLE					m_hSimConnect;
	FLOAT					m_faPHB_Buffer[3];
	INT						m_iQuit;
	BOOL					m_bSimConnectSuccessFlag;
	//
	FLOAT					m_faKnots[8];
	FLOAT					m_faPitchCoefs[4];
	FLOAT					m_faRollCoefs[4];
	FLOAT					m_faYawCoefs[4];
	INT						m_FnvalTiming;
	//P3D_Attitude			m_sAttitude;
	//P3D_Attitude			*psAttitude;
	float					m_fPreHead_w[5];

	DataToHost				m_sReturnedDataFromExpansion;
	DataToExpansion			m_sDataToExpansion;
	ExpansionToHost			m_sExpansionToHost;
	int P3D_DataProcess();
	int P3D_ExternalControlDataProcess(DataToHost tsDataToHost);
	//

	//DIRT3
public:
	SimtoolsData m_sSimtoolsData;

	int DIRT3_DataProcess();
	//PCAR2
public:
	bool m_Pcar2RunStatus;
	HANDLE fileHandle;
	SharedMemory* sharedData;
	unsigned int updateIndex;
	unsigned int indexChange;
	int PCAR2_DataProcess();
	// //检测Pcar2游戏是否运行
	bool Pcar2IsStartUp();
	int Pcar2SharedMemoryInit();
public:
	afx_msg void OnRcancel();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnShowDlg();
	afx_msg void OnToMiddle();
	afx_msg void OnToBottom();
	afx_msg void OnEndSession(BOOL bEnding);

};

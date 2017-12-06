
// GamePlatform.h : main header file for the PROJECT_NAME application
//

#pragma once
#include "ConnectToController.h"
#include "ConnectToAttitudeSource.h"
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

//#define  USE_EXTERNAL_CONTROL
#define USE_DOF
//#define SHOW_DLG
#define MOTUS_LOCK
#define LIMIT_GAME_TIME

// CGamePlatformApp:
// See GamePlatform.cpp for the implementation of this class
//

class CGamePlatformApp : public CWinApp
{
public:
	CGamePlatformApp();

	MSG msg;
// Overrides
public:
	virtual BOOL InitInstance();

	
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CGamePlatformApp theApp;
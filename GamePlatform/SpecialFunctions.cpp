#include "stdafx.h"
#include "SpecialFunctions.h"


CSpecialFunctions::CSpecialFunctions()
{
}


CSpecialFunctions::~CSpecialFunctions()
{
}


INT CSpecialFunctions::GetIntDataFromConfigFile(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	TCHAR tcExePath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, tcExePath, MAX_PATH);											//retrieves the path of the executable file of the current process,save to tcExePath

	TCHAR *pFind = _tcsrchr(tcExePath, '\\');
	if (pFind == NULL)
	{
		AfxMessageBox(_T("Fail to get the path of the executable file of the current process"));
		return 0;
	}
	*pFind = '\0';
	CString szIniPath = tcExePath;
	szIniPath += "\\";

	szIniPath += lpFileName;

	return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, lpFileName);
}

DWORD CSpecialFunctions::GetStringFromConfigFile(
	_In_  LPCTSTR lpAppName,
	_In_  LPCTSTR lpKeyName,
	_In_  LPCTSTR lpDefault,
	_Out_ LPTSTR  lpReturnedString,
	_In_  DWORD   nSize,
	_In_  LPCTSTR lpFileName
	)
{
	TCHAR tcExePath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, tcExePath, MAX_PATH);											//retrieves the path of the executable file of the current process,save to tcExePath

	TCHAR *pFind = _tcsrchr(tcExePath, '\\');
	if (pFind == NULL)
	{
		AfxMessageBox(_T("Fail to get the path of the executable file of the current process"));
		return 0;
	}
	*pFind = '\0';
	CString szIniPath = tcExePath;
	szIniPath += "\\";

	szIniPath +=lpFileName;

	DWORD bytenumber = GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, szIniPath);
	if (0x02 == GetLastError())
	{
		AfxMessageBox(TEXT("ConfigFile is not found!\r\nPlease Check!"));
		return -1;
	}
	else
	{
		return bytenumber;
	}
}

BOOL CSpecialFunctions::WriteStringToConfigFile(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	TCHAR tcExePath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, tcExePath, MAX_PATH);											//retrieves the path of the executable file of the current process,save to tcExePath

	TCHAR *pFind = _tcsrchr(tcExePath, '\\');
	if (pFind == NULL)
	{
		AfxMessageBox(_T("Fail to get the path of the executable file of the current process"));
		return 0;
	}
	*pFind = '\0';
	CString szIniPath = tcExePath;
	szIniPath += "\\";

	szIniPath += lpFileName;


	return WritePrivateProfileString(lpAppName, lpKeyName, lpString, szIniPath);
}

//Èý´ÎBÑùÌõ²åÖµ¼ÆËã
//knots[]:½ÚµãÏòÁ¿£¬³¤¶È±ØÐëÎª8
//coefs[]:¿ØÖÆ¶¨µã£¬³¤¶È±ØÐëÎª4
//x:²åÖµµã
//·µ»ØÖµ:¶ÔÓ¦ÓÚxµÄ

float CSpecialFunctions::fnval(float knots[], float coefs[], float x)
{
	int k = 4;
	int n = 4;
	int index = 0;

	//de-boor
	float tx[6];  //tx[]µÄ³¤¶ÈÎª2*(k-1)£»
	float b[4];   //b[]µÄ³¤¶ÈÎªk£»

	//²åÖµµãx±ØÐëÔÚ½ÚµãÐòÁÐ[k,n-1]Ö®¼ä
	if (x < knots[k - 1])
		x = knots[k - 1];
	else if (x > knots[n])
		x = knots[n];

	for (int i = k - 1; i < n; i++)
	{
		if (x < knots[i])
		{
			index = i;
			break;
		}
		else if (x >= knots[n - 1])
		{
			index = n;
		}
	}

	for (int i = 0; i < 2 * (k - 1); i++)
	{
		tx[i] = knots[i + 1 - k + index];
		tx[i] -= x;
	}


	for (int i = 0; i < k; i++)
	{
		b[i] = coefs[i - k + index];
	}

	for (int r = 0; r < k - 1; r++)
	{
		for (int i = 0; i < k - r - 1; i++)
		{
			b[i] = (tx[i + k - 1] * b[i] - tx[i + r] * b[i + 1]) / (tx[i + k - 1] - tx[i + r]);
		}
	}
	return b[0];
}

//限速，限加速度程序
float	CSpecialFunctions::LimitVelAndACC(MotionParaList *tspMotionParaList)
{
	float tfCurVel=0.0f;							//current velocity
	float tfPreVel=0.0f;							//the last velocity
	float tfCurAcc=0.0f;							//current acceleration
	float tfKpVel=0.95;
	float tfKpAcc = 0.7;
	tfCurVel = tspMotionParaList->fDesPos - tspMotionParaList->fPrePos;
	tfPreVel = tspMotionParaList->fPrePos - tspMotionParaList->fPrePrePos;
	tfCurAcc = tfCurVel - tfPreVel;
	
	if ((tspMotionParaList->fMaxAngularAcc > fabsf(tfCurAcc)) || (tspMotionParaList->fMaxAngularAcc <=1.0e-06))
	{
		/*if (tspMotionParaList->fMaxAngularVel <= fabsf(tfCurVel))
		{
			tspMotionParaList->fDesPos = tspMotionParaList->fPrePos*tfKpVel + tspMotionParaList->fPrePos*(1 - tfKpVel);
		}*/
		if (tspMotionParaList->fMaxAngularVel <= tfCurVel)
		{
			tspMotionParaList->fDesPos = tspMotionParaList->fPrePos + tspMotionParaList->fMaxAngularVel;
		}
		else if (-tspMotionParaList->fMaxAngularVel >= tfCurVel)
		{
			tspMotionParaList->fDesPos = tspMotionParaList->fPrePos - tspMotionParaList->fMaxAngularVel;
		}
	}
	else \
	/*{
		tspMotionParaList->fDesPos = tspMotionParaList->fPrePos*tfKpAcc+ tspMotionParaList->fPrePos*(1-tfKpAcc);
	}*/
	if (tspMotionParaList->fMaxAngularAcc <= tfCurAcc)
	{
		tspMotionParaList->fDesPos = tspMotionParaList->fPrePos + tspMotionParaList->fMaxAngularAcc / 10.0;
	}
	else if (-tspMotionParaList->fMaxAngularAcc >= tfCurAcc)
	{
		tspMotionParaList->fDesPos = tspMotionParaList->fPrePos - tspMotionParaList->fMaxAngularAcc / 10.0;
	}
	if (tspMotionParaList->fDesPos >= tspMotionParaList->fMaxDesPos)
	{
		tspMotionParaList->fDesPos = tspMotionParaList->fMaxDesPos;
	}
	else if (tspMotionParaList->fDesPos <= -tspMotionParaList->fMaxDesPos)
	{
		tspMotionParaList->fDesPos = -tspMotionParaList->fMaxDesPos;
	}
	tspMotionParaList->fPrePrePos = tspMotionParaList->fPrePos;
	tspMotionParaList->fPrePos = tspMotionParaList->fDesPos;

	return tspMotionParaList->fDesPos;
}
#pragma once
#include "MT_AsyncSocket.h"

enum DOF_state
{
	dof_stop = 0,			//
	dof_sys_moving = 1,		//
	dof_neutral = 2,		//
	dof_working = 3,		//
	dof_setconf = 3,		//
	dof_select = 8,
	dof_check_id = 55,
	dof_closed = 253,
	dof_emg = 254,
	dof_err = 255
};
enum M_nCmd
{
	S_CMD_RUN = 0,									//正常运行
	S_CMD_Check = 1,
	S_CMD_Back2MID = 2,								// 底回中立位
	S_CMD_ToMerg = 3,								//	紧急停机
	S_CMD_ToWork = 4,								// 握手协议
	S_CMD_JOG = 5,									//单缸手动	
	S_CMD_Work = 6,									//由低位上升到中位
	S_CMD_Stop = 7,									//由中位回落到低位
	S_CMD_ChaConf = 8,								//配置驱动器信息
	S_CMD_HOM = 9,
	S_CMD_JOYCTRL = 101,
	S_CMD_GAMECTRL = 102,
	S_CMD_GAMESTARTUP=120,

	S_CMD_ENABLE_RUN=168,
};

struct DataToDOF
{
	BYTE nCheckID;
	BYTE nCmd;
	BYTE nAct;
	BYTE nReserved;									//保留

	float DOFs[6];									//{横摇，纵倾，航向，前向，侧向，升降	}
	float Vxyz[3];									//{前向，侧向，升降}，向右为正，向下为正
	float Axyz[3];		//...
};

struct DataToHost
{
	BYTE nCheckID;
	BYTE nDOFStatus;
	BYTE nRev0;										//需要使用
	BYTE nRev1;										//需要使用

	float attitude[6];								//需要使用，RotorTrim，MixtureControl
	float para[6];									
	float motor_code[6];							
};

class CConnectToController :
	public CMT_AsyncSocket
{
public:
	CConnectToController();
	~CConnectToController();

	virtual void OnReceive(int nErrorCode);
	//parameter
	const int m_MaxDelay = 6000;					//60000*10ms
	DataToDOF m_preDataToDof;
	DataToDOF m_sToDOFBuf;
	DataToDOF m_sDataFromMainControlToDof;//m_sToDOFBuf2;
	DataToHost	m_sReturnedDataFromDOF;
	TCHAR m_tcaControllerIP[17];
	UINT m_nControllerPort;


	//function

	int DOF_UpToMedian();
	int DOF_ToRun();										//enter the state of running
	int DOF_ToMedian();										//return to the state of median from any state
	int DOF_ToBottom();

};


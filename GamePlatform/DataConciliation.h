// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DATACONCILIATION_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// DATACONCILIATION_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#pragma once
#ifndef DataConciliation_H
#define DataConciliation_H

#ifdef DATACONCILIATION_EXPORTS
#define DATACONCILIATION_API __declspec(dllexport)
#else
#define DATACONCILIATION_API __declspec(dllimport)
#endif

#include <Afxsock.h>

typedef struct DataStruct
{
	unsigned  char checkID; //发送给那个平台
	unsigned  char valid;   //命令有效
	unsigned  char select;  //影片或游戏选择
	unsigned  char StartPause; //开始暂停
	float  kVaule[6];//K值
	float  pVaule[6];//P值
	float  zVaule[6];//Z值
	DataStruct()
	{
		checkID = 55; //发送给那个平台
		valid = 0;   //命令有效 初始值为0  1为影片或游戏切换 2为游戏开始暂停结束有效 3为数据有效查询  4数据有效更改数据
		select = 0;  //影片或游戏选择 初始值为0 有效值为 1、2、3、、、
		StartPause = 0; //开始暂停 初始值为0  1开始 2 暂停 3结束
		for (int i = 0; i < 6; i++)
		{
			kVaule[i] = 0.0f;//K值
			pVaule[i] = 0.0f;//P值
			zVaule[i] = 0.0f;//Z值
		}
	}
}MDataStruct;

// 此类是从 DataConciliation.dll 导出的
class DATACONCILIATION_API CDataConciliation {
public:
	CDataConciliation(void);//构造函数
	~CDataConciliation();//析构函数
	bool socketInit(char strIP[30], int port);//初始化函数
	void setRemote(char strIP[30], int port);//设置远程IP和端口
	bool sendData(void);//发送数据
	void setData(MDataStruct &Vaule);//设置数据
	void getDataStruct(MDataStruct *inVaule);
private:
	static UINT ThreadProc(LPVOID pParam);//线程函数
	struct sockaddr_in remote_addr;//远程ip和端口号
	int sockfd;  //socket
	bool runFlag;//线程运行标志
	HANDLE hThread;//线程句柄
	MDataStruct mMDataStruct;//接收到的数据
};
#endif
// RunGateSpeedManage1.h: RunGateSpeedManage1 DLL 的主标头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号


// CRunGateSpeedManage1App
// 有关此类实现的信息，请参阅 RunGateSpeedManage1.cpp
//

class CRunGateSpeedManage1App : public CWinApp
{
public:
	CRunGateSpeedManage1App();

// 重写
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

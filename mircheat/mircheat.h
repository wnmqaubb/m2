// mircheat1.h: mircheat1 DLL 的主标头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// 主符号
#include "mircheatDlg.h"

// Cmircheat1App
// 有关此类实现的信息，请参阅 mircheat.cpp
//

class CmircheatApp : public CWinApp
{
public:
	CmircheatApp();

// 重写
public:
	virtual BOOL InitInstance();
    CmircheatDlg* dlg;
	DECLARE_MESSAGE_MAP()
};

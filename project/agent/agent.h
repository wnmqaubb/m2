
// agent.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// 主符号
#include <dump.hpp>
#include <log_client_sdk.h>

// CagentApp:
// 有关此类的实现，请参阅 agent.cpp
//

class CagentApp : public CWinApp, DumpService
{
public:
	CagentApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CagentApp theApp;

namespace Global
{
    extern ProtocolPolicy g_policy;
    extern std::wstring g_error_text;
    bool AddPolicy(SubProto::PolicyType type, SubProto::PunishType punish_type, std::wstring config, std::wstring comment=L"");
    void LoadConfig();
    void SaveConfig();
    bool FindPolicy(SubProto::PolicyType type, SubProto::Policy& policy_out);
    bool FindPolicy(SubProto::PolicyType type, std::vector<SubProto::Policy>& policy_out);
    CString ConvertToString(SubProto::PolicyType type);
    CString ConvertToString(SubProto::PunishType type);

}

#define GLOG(x,...)  ((CagentDlg*)theApp.GetMainWnd())->m_LogDlg.log(x,__VA_ARGS__)
#ifdef _DEBUG
#define CONFIG_APP_NAME "内部测试"
#define CONFIG_WEBSITE "www.baidu.com"
#endif
#ifdef BUILD_RELEASE
#define CONFIG_APP_NAME "二进制"
#define CONFIG_WEBSITE "www.7522.xyz"
#endif
#ifdef BUILD_RELEASE2
#define CONFIG_APP_NAME "审判"
#define CONFIG_WEBSITE "www.pm76.com"
#endif
#define INTERVAL_DETECTION_RULE 30
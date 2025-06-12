#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号
#include "../Gate/ObserverClientGroupImpl.h"
#include "../Service/SubServicePackage.h"
#include <memory>


inline std::shared_ptr<spdlog::logger> slog;
// 使用枚举类代替普通枚举
enum class AnchorStyle : unsigned int {
    LEFT = 0x01,
    TOP = 0x02,
    RIGHT = 0x04,
    BOTTOM = 0x08
};

// 重载|运算符用于组合锚定方式
constexpr AnchorStyle operator|(AnchorStyle a, AnchorStyle b) {
    return static_cast<AnchorStyle>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

// 重载&运算符用于检查锚定方式
constexpr unsigned int operator&(AnchorStyle a, AnchorStyle b) {
    return static_cast<unsigned int>(a) & static_cast<unsigned int>(b);
}

struct ControlLayoutInfo {
    CRect originalRect;
    UINT nID;
    AnchorStyle anchor;
};
typedef enum _SETTIMEOUT_ID
{
	TIMER_ID_RELOAD_GAMER_LIST = 1,
	TIMER_ID_POLL_WORK_ID,
	TIMER_ID_CHILD_SERIVCE_ID,
	TIMER_ID_UPDATE_UAERNAME,
	TIMER_ID_QUERY_VMP_EXPIRE,
}SETTIMEOUT_ID;
#define GATE_ADMIN_POLICY_ID 689000
#define GATE_POLICY_ID 688000
class CGateFDlg;
class CGamesDlg;

class CGateFApp : public CWinAppEx
{
public:
	CGateFApp();

// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	void OnServiceStart();
	void OnServiceStop();
	void OnServiceStop1();
	virtual BOOL OpenFolderAndSelectFile(CString szPath);
	virtual CGateFDlg* GetMainFrame();
	std::string ReadLicense();
	std::string ReadAuthKey();
    CString ReadExpire();
	void OpenConfig();
	void SaveConfig();
	// 实现
	CHAR  m_ExeDir[MAX_PATH];
	CString m_cCfgPath;
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	CObserverClientGroupImpl m_ObServerClientGroup;
	asio::io_service m_WorkIo;
	CDocument* m_ConfigDoc = nullptr;
	HANDLE m_childpHandle;
	//CConfig m_wndConfig;
	bool is_parent_gate = true;
	bool is_service_stauts = true;
	std::unique_ptr<ProtocolS2CPolicy> m_cfg;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnServiceSettings();
	afx_msg void InitConfig();
};

extern CGateFApp theApp;

template <typename... Args>
void LogPrint(int type, LPCTSTR format, Args... args)
{
	CWnd* lpMainWnd = theApp.m_pMainWnd;
	if (!lpMainWnd)
		return;
	ASSERT(lpMainWnd);
	ASSERT(lpMainWnd->IsKindOf(RUNTIME_CLASS(CGateFDlg)));
	reinterpret_cast<CGateFDlg*>(lpMainWnd)->m_logs_dlg->LogPrint(type, format, args...);
}

static CString ConvertToString(PunishType type)
{
	std::map<PunishType, LPCTSTR> mPunishType = {
		{ENM_PUNISH_TYPE_KICK,TEXT("退出游戏")},
		{ENM_PUNISH_TYPE_NO_OPEARATION,TEXT("不处理")},
		{ENM_PUNISH_TYPE_SUPER_WHITE_LIST,TEXT("白名单")},
		{ENM_PUNISH_TYPE_BAN_MACHINE,TEXT("封机器")},
		{ENM_PUNISH_TYPE_ENABLE,TEXT("启用")},
		{ENM_PUNISH_TYPE_DISABLE,TEXT("禁用")},
	};
	if (mPunishType.find(type) != mPunishType.end())
	{
		return mPunishType[type];
	}
	else
	{
		return TEXT("未定义");
	}
}

static PunishType ConvertToPunishType(CString punishname)
{
	std::map<CString, PunishType> mPunishType = {
		{ TEXT("退出游戏"), ENM_PUNISH_TYPE_KICK },
		{ TEXT("不处理"), ENM_PUNISH_TYPE_NO_OPEARATION },
		{ TEXT("白名单"), ENM_PUNISH_TYPE_SUPER_WHITE_LIST },
		{ TEXT("封机器"), ENM_PUNISH_TYPE_BAN_MACHINE },
		{ TEXT("启用"), ENM_PUNISH_TYPE_ENABLE },
		{ TEXT("禁用"), ENM_PUNISH_TYPE_DISABLE },
	};
	if (mPunishType.find(punishname) != mPunishType.end())
	{
		return mPunishType[punishname];
	}
	else
	{
		return (PunishType)0;
	}
}

static CString ConvertToString(PolicyType type)
{
	std::map<PolicyType, LPCTSTR> mPolicyType = {
		{ENM_POLICY_TYPE_MODULE_NAME,TEXT("模块名检测")},
		{ENM_POLICY_TYPE_PROCESS_NAME,TEXT("进程名检测")},
		{ENM_POLICY_TYPE_PROCESS_NAME_AND_SIZE,TEXT("进程名+大小检测")},
		{ENM_POLICY_TYPE_FILE_NAME,TEXT("文件路径")},
		{ENM_POLICY_TYPE_WINDOW_NAME,TEXT("窗口名")},
		{ENM_POLICY_TYPE_MACHINE,TEXT("机器码")},
		{ENM_POLICY_TYPE_MULTICLIENT,TEXT("多开限制")},
		{ENM_POLICY_TYPE_SHELLCODE,TEXT("云代码")},
		{ENM_POLICY_TYPE_SCRIPT,TEXT("脚本")},
		{ENM_POLICY_TYPE_THREAD_START,TEXT("线程特征")},
		{ENM_POLICY_TYPE_BACK_GAME,TEXT("延时小退")},
		{ENM_POLICY_TYPE_EXIT_GAME,TEXT("延时大退")},
		{ENM_POLICY_TYPE_ACTION_SPEED_WALK,TEXT("走路间隔")},
		{ENM_POLICY_TYPE_ACTION_SPEED_HIT,TEXT("攻击间隔")},
		{ENM_POLICY_TYPE_ACTION_SPEED_SPELL,TEXT("魔法间隔")}
	};
	if (mPolicyType.find(type) != mPolicyType.end())
	{
		return mPolicyType[type];
	}
	else
	{
		return TEXT("内部策略");
	}
}

static PolicyType ConvertToPolicyType(CString policyname)
{
	std::map<CString, PolicyType> mPolicyType = {
		{ TEXT("模块名检测"), ENM_POLICY_TYPE_MODULE_NAME },
		{ TEXT("进程名检测"), ENM_POLICY_TYPE_PROCESS_NAME },
		{ TEXT("进程名+大小检测"), ENM_POLICY_TYPE_PROCESS_NAME_AND_SIZE },
		{ TEXT("文件路径"), ENM_POLICY_TYPE_FILE_NAME },
		{ TEXT("窗口名"), ENM_POLICY_TYPE_WINDOW_NAME },
		{ TEXT("机器码"), ENM_POLICY_TYPE_MACHINE },
		{ TEXT("多开限制"), ENM_POLICY_TYPE_MULTICLIENT },
		{ TEXT("云代码"), ENM_POLICY_TYPE_SHELLCODE },
		{ TEXT("脚本"), ENM_POLICY_TYPE_SCRIPT },
		{ TEXT("线程特征"), ENM_POLICY_TYPE_THREAD_START },
		{ TEXT("延时小退"), ENM_POLICY_TYPE_BACK_GAME },
		{ TEXT("延时大退"), ENM_POLICY_TYPE_EXIT_GAME },
		{ TEXT("走路间隔"), ENM_POLICY_TYPE_ACTION_SPEED_WALK },
		{ TEXT("攻击间隔"), ENM_POLICY_TYPE_ACTION_SPEED_HIT },
		{ TEXT("魔法间隔"), ENM_POLICY_TYPE_ACTION_SPEED_SPELL }
	};
	if (mPolicyType.find(policyname) != mPolicyType.end())
	{
		return mPolicyType[policyname];
	}
	else
	{
		return (PolicyType)0;
	}
}
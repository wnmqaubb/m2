
// Gate.h: Gate 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // 主符号

#include "OutputDlg.h"
#include "MainFrm.h"
#include "ObserverClientGroupImpl.h"
#include "CConfig.h"

// CGateApp:
// 有关此类的实现，请参阅 Gate.cpp
//

class CDocTemplateMgr
{
public:
    template <class T = CMultiDocTemplate>
    T* Find(const std::string& cTemplateName)
    {
        if (m_DocTemplates.find(cTemplateName) == m_DocTemplates.end())
        {
            return nullptr;
        }
        return (T*)m_DocTemplates[cTemplateName];
    }
    template <class T = CMultiDocTemplate>
    T* Add(const std::string& cTemplateName, UINT nIDResource, CRuntimeClass* pDocClass,
        CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
    {
        if (m_DocTemplates.find(cTemplateName) == m_DocTemplates.end())
        {
            m_DocTemplates[cTemplateName] = new T(nIDResource,
                pDocClass,
                pFrameClass, // 自定义 MDI 子框架
                pViewClass);
        }
        return (T*)m_DocTemplates[cTemplateName];
    }
private:
    std::map<std::string, void*> m_DocTemplates;
};

class CGateApp : public CWinAppEx
{
public:
	CGateApp() noexcept;
    virtual BOOL OpenFolderAndSelectFile(CString szPath);
    std::string ReadLicense();
    std::string ReadAuthKey();
    void ConnectionLicenses();
    virtual CDocTemplateMgr& GetDocTemplateMgr() { return m_DocTemplatesMgr; }
protected:
    CDocTemplateMgr m_DocTemplatesMgr;
// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
    
// 实现
    CHAR  m_ExeDir[MAX_PATH];
    CString m_cCfgPath;
    UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
    CObserverClientGroupImpl m_ObServerClientGroup;
    asio::io_service m_WorkIo;
    CDocument* m_ConfigDoc = nullptr;
    HANDLE m_childpHandle;
    CConfig m_wndConfig;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
    virtual CMainFrame* GetMainFrame();
    afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnServiceStart();
	afx_msg void OnServiceStop();
    afx_msg void OnAppExit();
    afx_msg void OnServiceSettings();
    afx_msg void OnConfig();
    
};

template <typename... Args>
void LogPrint(int type, LPCTSTR format, Args... args)
{
    CWnd* lpMainWnd = theApp.m_pMainWnd;
    if (!lpMainWnd)
        return;
    ASSERT(lpMainWnd);
    ASSERT(lpMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    reinterpret_cast<CMainFrame*>(lpMainWnd)->GetOutputWindow().LogPrint(type, format, args...);
}

#ifndef GATE_EXPORT
#define GATE_EXPORT __declspec(dllimport)
#else
extern CGateApp theApp;
#endif

GATE_EXPORT CGateApp* GetGateAppInstance();


static CString ConvertToString(PunishType type)
{
    std::map<PunishType, LPCTSTR> mPunishType = {
        {ENM_PUNISH_TYPE_KICK,TEXT("退出游戏")},
        //{ENM_PUNISH_TYPE_BSOD,TEXT("蓝屏")},
        {ENM_PUNISH_TYPE_NO_OPEARATION,TEXT("不处理")},
        {ENM_PUNISH_TYPE_SUPER_WHITE_LIST,TEXT("白名单")},
        {ENM_PUNISH_TYPE_BAN_MACHINE,TEXT("封机器")},
        {ENM_PUNISH_TYPE_SCREEN_SHOT,TEXT("截图")},
        {ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,TEXT("截图+退出游戏")},
        //{ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD,TEXT("截图+蓝屏")},
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
        //{ TEXT("蓝屏"), ENM_PUNISH_TYPE_BSOD },
        { TEXT("不处理"), ENM_PUNISH_TYPE_NO_OPEARATION },
        { TEXT("白名单"), ENM_PUNISH_TYPE_SUPER_WHITE_LIST },
        { TEXT("封机器"), ENM_PUNISH_TYPE_BAN_MACHINE },
        { TEXT("截图"), ENM_PUNISH_TYPE_SCREEN_SHOT },
        { TEXT("截图+退出游戏"), ENM_PUNISH_TYPE_SCREEN_SHOT_KICK },
        //{ TEXT("截图+蓝屏"), ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD },
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
        {ENM_POLICY_TYPE_FILE_NAME,TEXT("文件路径")},
        {ENM_POLICY_TYPE_WINDOW_NAME,TEXT("窗口名")},
        {ENM_POLICY_TYPE_MACHINE,TEXT("机器码")},
        {ENM_POLICY_TYPE_MULTICLIENT,TEXT("多开限制")},
        /*{ENM_POLICY_TYPE_SHELLCODE,TEXT("云代码")},*/
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
    std::map<CString,PolicyType> mPolicyType = {
        { TEXT("模块名检测"), ENM_POLICY_TYPE_MODULE_NAME },
        { TEXT("进程名检测"), ENM_POLICY_TYPE_PROCESS_NAME },
        { TEXT("文件路径"), ENM_POLICY_TYPE_FILE_NAME },
        { TEXT("窗口名"), ENM_POLICY_TYPE_WINDOW_NAME },
        { TEXT("机器码"), ENM_POLICY_TYPE_MACHINE },
        { TEXT("多开限制"), ENM_POLICY_TYPE_MULTICLIENT },
        /*{ TEXT("云代码"), ENM_POLICY_TYPE_SHELLCODE },*/
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
<<<<<<< HEAD
﻿#pragma once


// CConfig 对话框

class CConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CConfig)

public:
	CConfig(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CConfig();
    void ShowConfigDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONFIG_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    BOOL m_lazy_back_enable;
    BOOL m_lazy_exit_enable;
    BOOL m_speed_walk_enable;
    BOOL m_speed_hit_enable;
    BOOL m_speed_spell_enable;
    uint32_t m_lazy_back;
    uint32_t m_lazy_exit;
    uint32_t m_speed_walk;
    uint32_t m_speed_hit;
    uint32_t m_speed_spell;
    afx_msg void OnClose();
    BOOL m_can_lazy_back_exit_enable;
};
=======
﻿#pragma once


// CConfig 对话框

class CConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CConfig)

public:
	CConfig(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CConfig();
    void ShowConfigDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONFIG_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    BOOL m_lazy_back_enable;
    BOOL m_lazy_exit_enable;
    BOOL m_speed_walk_enable;
    BOOL m_speed_hit_enable;
    BOOL m_speed_spell_enable;
    uint32_t m_lazy_back;
    uint32_t m_lazy_exit;
    uint32_t m_speed_walk;
    uint32_t m_speed_hit;
    uint32_t m_speed_spell;
    afx_msg void OnClose();
    BOOL m_can_lazy_back_exit_enable;
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

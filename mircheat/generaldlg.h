<<<<<<< HEAD
﻿#pragma once
//#include "global.h"
#include <afxdialogex.h>


// GeneralDlg 对话框

class GeneralDlg : public CDialogEx
{
	DECLARE_DYNAMIC(GeneralDlg)

public:
	GeneralDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~GeneralDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_GENERAL_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    CSliderCtrl m_general_move_speed_slider;
    CSliderCtrl m_general_attack_speed_slider;
    CSliderCtrl m_general_magic_speed_slider;
    CSliderCtrl m_general_move_interval_slider;
    CSliderCtrl m_general_attack_interval_slider;
    CSliderCtrl m_general_magic_interval_slider;
    CSliderCtrl m_general_speed_change_slider;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedGeneralMoveSpeed();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    int m_general_move_speed_static;
    int m_general_attack_speed_static;
    int m_general_magic_speed_static;
    int m_general_move_interval_static;
    int m_general_attack_interval_static;
    int m_general_magic_interval_static;
    CString m_general_speed_change_static_str;
    float m_general_speed_change_static;
    afx_msg void OnDeltaposGeneralMoveSpeed(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedGeneralAttackSpeed();
    afx_msg void OnBnClickedGeneralMagicSpeed();
    afx_msg void OnBnClickedGeneralMoveInterval();
    afx_msg void OnBnClickedGeneralAttackInterval();
    afx_msg void OnBnClickedGeneralMagicInterval();
    afx_msg void OnDeltaposGeneralAttackSpeed(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralMagicSpeed(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralMoveInterval(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralAttackInterval(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralMagicInterval(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralSpeedChange(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedCheck9();
    afx_msg void OnBnClickedCheck1();
};
=======
﻿#pragma once
//#include "global.h"
#include <afxdialogex.h>


// GeneralDlg 对话框

class GeneralDlg : public CDialogEx
{
	DECLARE_DYNAMIC(GeneralDlg)

public:
	GeneralDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~GeneralDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_GENERAL_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    CSliderCtrl m_general_move_speed_slider;
    CSliderCtrl m_general_attack_speed_slider;
    CSliderCtrl m_general_magic_speed_slider;
    CSliderCtrl m_general_move_interval_slider;
    CSliderCtrl m_general_attack_interval_slider;
    CSliderCtrl m_general_magic_interval_slider;
    CSliderCtrl m_general_speed_change_slider;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedGeneralMoveSpeed();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    int m_general_move_speed_static;
    int m_general_attack_speed_static;
    int m_general_magic_speed_static;
    int m_general_move_interval_static;
    int m_general_attack_interval_static;
    int m_general_magic_interval_static;
    CString m_general_speed_change_static_str;
    float m_general_speed_change_static;
    afx_msg void OnDeltaposGeneralMoveSpeed(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedGeneralAttackSpeed();
    afx_msg void OnBnClickedGeneralMagicSpeed();
    afx_msg void OnBnClickedGeneralMoveInterval();
    afx_msg void OnBnClickedGeneralAttackInterval();
    afx_msg void OnBnClickedGeneralMagicInterval();
    afx_msg void OnDeltaposGeneralAttackSpeed(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralMagicSpeed(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralMoveInterval(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralAttackInterval(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralMagicInterval(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposGeneralSpeedChange(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedCheck9();
    afx_msg void OnBnClickedCheck1();
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

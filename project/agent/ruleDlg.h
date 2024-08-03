#pragma once
#include <WinNT.h>


// ProcessModuleRule 对话框
class CagentDlg;
class RuleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(RuleDlg)

public:
	RuleDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RuleDlg();

    // 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_RULE_LIST_DLG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_json_config;
	CagentDlg *m_agentDlg;
	virtual BOOL OnInitDialog();
    void UpdateConfigList();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    CListCtrl m_PolicyList;
    afx_msg void OnBnClickedButtonAdd();
    afx_msg void OnBnClickedButtonEdit();
    CComboBox m_PolicyTypeComboBox;
    CComboBox m_PunishTypeComboBox;
    CEdit m_ConfigEdit;
    uint32_t m_CurrentSeletedPolicyId;
    std::map<SubProto::PunishType, int> m_PunishTypeListMap;
    afx_msg void OnNMClickListConfig(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnClose();
    afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
    void ClearPunishTypeList();
    void AddPunishTypeToMap(SubProto::PunishType type, int index);
    int GetPunishTypeListIndex(SubProto::PunishType type);
    afx_msg void OnCbnDropdownComboPunishType();
    afx_msg void OnCbnDropdownComboPolicyType();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    CString m_comment;
    CString m_search_text;
    CString m_search_text_old;
    MFCUtil mfcutil;
    afx_msg void OnBnClickedButtonOnlineGamerSearch();
};

#pragma once
#include "afxdialogex.h"
#include <vector>
#include "GateF.h"


// CPoliceDlg 对话框
class CGateFDlg;

class CPoliceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPoliceDlg)

public:
	CPoliceDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPoliceDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_POLICES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	void OnInitialUpdate();
	void OnSize(UINT nType, int cx, int cy);
protected:
	void AdjustLayout();
	void InitConfigSettingView();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConfigAdd();
	afx_msg void OnConfigDel();
	afx_msg void OnConfigSave();
    CSize m_originalSize; // 添加这行
    std::vector<ControlLayoutInfo> m_layoutInfos;
	DECLARE_MESSAGE_MAP()
private:
	int m_nCurSelRow, m_nCurSelCol;
public:
	void OnSelectItem(int policy_id);
	uint32_t next_gm_policy_id(std::map<uint32_t, ProtocolPolicy>& policies);
	void RefreshViewList();
	CListCtrl m_list_polices; 
	CEdit m_editCtrl;
	CComboBox m_combo_policy_type;
	CComboBox m_combo_punish_type;
	virtual BOOL OnInitDialog();
	afx_msg void OnNMClickListPolices(NMHDR* pNMHDR, LRESULT* pResult);
	CFont* GetCellFont(int nRow, int nCol);
	void EditCell(int nRow, int nCol);
	void OnEnKillfocusEditCtrl();
	void OnCbnKillfocusComboPolicyType();
	void OnCbnKillfocusComboPunishType();
	void SetControlBorderColor(CWnd* pCtrl, COLORREF color);
	void OnCbnDropdownComboPunishType(CString policy_type);
	BOOL m_check_base_policy;
	BOOL m_check_better_policy;
	BOOL m_check_best_policy;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedRefreshPolicy();
	int m_policy_detect_interval;
	CSpinButtonCtrl m_spin_detect_interval;
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
};

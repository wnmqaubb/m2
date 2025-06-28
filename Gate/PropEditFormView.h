#pragma once
#include <afxcmn.h>



// PropEditFormView 窗体视图

class CPropEditFormView : public CFormView
{
	DECLARE_DYNCREATE(CPropEditFormView)
public:
	CPropEditFormView() noexcept;   
	virtual ~CPropEditFormView();
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROP_EDIT_VIEW };
#endif
    using CFormView::Create;
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

    void ClearPunishTypeList()
    {
        m_PunishTypeListMap.clear();
    }
    void AddPunishTypeToMap(PunishType type, int index)
    {
        m_PunishTypeListMap.emplace(std::make_pair(type, index));
    }
    int GetPunishTypeListIndex(PunishType type)
    {
        return m_PunishTypeListMap[type];
    }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_PolicyTypeComboBox;
    CComboBox m_PunishTypeComboBox;
    CEdit m_PolicyIdEdit;
    CRichEditCtrl m_PolicyConfigEdit;
    CEdit m_PolicyCommentEdit;
    bool m_CreateByAdmin;
    std::map<PunishType, int> m_PunishTypeListMap;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnCbnDropdownComboPolicyType();
    afx_msg void OnCbnDropdownComboPunishType();
    virtual void OnInitialUpdate();
    afx_msg void OnBnClickedButtonPolicyCommit();
};



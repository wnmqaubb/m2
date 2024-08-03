#pragma once



// SearchBar 窗体视图

class CSearchBar : public CFormView
{
	DECLARE_DYNCREATE(CSearchBar)

protected:
	CSearchBar();           // 动态创建所使用的受保护的构造函数
	virtual ~CSearchBar();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SEARCHBAR };
#endif
    using CFormView::Create;
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};



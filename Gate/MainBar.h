<<<<<<< HEAD
﻿#pragma once


// CMainBar 对话框
class CMainBar : public CFormView
{
    DECLARE_DYNCREATE(CMainBar)

protected:
	CMainBar();   // 标准构造函数
	virtual ~CMainBar();

public:
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINBAR };
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
    virtual void OnInitialUpdate();
    HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};
=======
﻿#pragma once


// CMainBar 对话框
class CMainBar : public CFormView
{
    DECLARE_DYNCREATE(CMainBar)

protected:
	CMainBar();   // 标准构造函数
	virtual ~CMainBar();

public:
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINBAR };
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
    virtual void OnInitialUpdate();
    HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

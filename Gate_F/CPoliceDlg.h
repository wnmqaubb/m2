#pragma once
#include "afxdialogex.h"


// CPoliceDlg 对话框

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

	DECLARE_MESSAGE_MAP()
};

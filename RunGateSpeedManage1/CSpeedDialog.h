#pragma once
#include "afxdialogex.h"


// CSpeedDialog 对话框

class CSpeedDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSpeedDialog)

public:
	CSpeedDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSpeedDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELAY_COMPENSATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

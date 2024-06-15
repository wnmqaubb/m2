// LogDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "LogDlg.h"
#include "afxdialogex.h"
#include <Richedit.h>


// LogDlg 对话框

IMPLEMENT_DYNAMIC(LogDlg, CDialogEx)

LogDlg::LogDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LOG, pParent)
{
	
}

LogDlg::~LogDlg()
{
}

void LogDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT21, m_RichEditLog);
    DDX_Control(pDX, IDC_CHECK_VERBOSE, m_VerboseCheck);
}

BEGIN_MESSAGE_MAP(LogDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_LOG, &LogDlg::OnBnClickedButtonClearLog)
END_MESSAGE_MAP()

void LogDlg::log(LPCTSTR format, ...)
{
	TCHAR lpBuffer[1024];
	va_list ap;
	va_start(ap, format);
#if _UNICODE
	vswprintf_s(lpBuffer, 1024 - 1, format, ap);
#else
	vsnprintf_s(lpBuffer, 1024 - 1, format, ap);
#endif
	va_end(ap);

	m_RichEditLog.GetScrollInfo(SB_VERT, &Info);
	if(Info.nPage+Info.nPos >= (UINT)Info.nMax)
	{
		m_RichEditLog.SetSel(-1, -1);
		m_RichEditLog.ReplaceSel(CString(lpBuffer) + TEXT("\r\n"));
	} else
	{
		int nOldFirstVisibleLine = m_RichEditLog.GetFirstVisibleLine();
		m_RichEditLog.SetSel(-1, -1);
		m_RichEditLog.ReplaceSel(CString(lpBuffer) + TEXT("\r\n"));
		int nNewFirstVisibleLine = m_RichEditLog.GetFirstVisibleLine();
		if(nOldFirstVisibleLine != nNewFirstVisibleLine)
		{
			m_RichEditLog.LineScroll(nOldFirstVisibleLine - nNewFirstVisibleLine);
		}
	}
}

void LogDlg::OnBnClickedButtonClearLog()
{
    m_RichEditLog.SetWindowText(TEXT(""));
}

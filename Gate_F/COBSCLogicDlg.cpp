// COBSCLogicDlg.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "afxdialogex.h"
#include "COBSCLogicDlg.h"


// COBSCLogicDlg 对话框

IMPLEMENT_DYNAMIC(COBSCLogicDlg, CDialogEx)

COBSCLogicDlg::COBSCLogicDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LOG_OBSERVER_CLIENT, pParent)
{

}

COBSCLogicDlg::~COBSCLogicDlg()
{
}

void COBSCLogicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT2_LOG_OBC_LOGIC, m_log_observer_client);
}


BEGIN_MESSAGE_MAP(COBSCLogicDlg, CDialogEx)
END_MESSAGE_MAP()


// COBSCLogicDlg 消息处理程序

// 添加日志信息
void COBSCLogicDlg::AddLog(const CString& log_txt, COLORREF txt_color)
{
	if (m_log_observer_client.GetLineCount() > 5000) {
		m_log_observer_client.SetSel(0, -1);
		m_log_observer_client.ReplaceSel(_T(""));
	}
	CHARFORMAT2 cf;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_SIZE /*| CFM_FACE | CFM_BOLD | CFM_ITALIC;*/;
	cf.yHeight = 20 * 10;
	cf.crTextColor = txt_color; // 设置日志文本颜色为蓝色

	int endPos = m_log_observer_client.GetWindowTextLength();
	m_log_observer_client.SetSel(endPos, endPos);
	m_log_observer_client.SetSelectionCharFormat(cf);
	m_log_observer_client.ReplaceSel(log_txt + _T("\r\n"));
	// 确保滚动条滚动到底部
	m_log_observer_client.PostMessage(WM_VSCROLL, SB_BOTTOM, -1);
}

BOOL COBSCLogicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 初始化富文本编辑控件

	// 设置富文本编辑控件的属性
	//m_log_observer_client.SetWindowTextW(_T("[Event]21:35:22|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:封机器 处罚原因:封机器码|178BFBFF00A50F00|00-50-56-C0-00-01|1142814342\r\n"));
	m_log_observer_client.SetBackgroundColor(false, RGB(0, 0, 0));
	//SetTextColor(RGB(0, 255, 0));
	/*AddLog(L"33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(255, 0, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 255, 0));
	AddLog(L"[Event]12:27:27|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:退出游戏 处罚原因:外挂程序为:TXPlatfor.exe|", RGB(0, 0, 255));*/
	return TRUE;
}
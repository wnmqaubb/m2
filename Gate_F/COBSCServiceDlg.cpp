// COBSCService.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "afxdialogex.h"
#include "COBSCServiceDlg.h"
#include "GateFDlg.h"
#include <memory>


// COBSCService 对话框

IMPLEMENT_DYNAMIC(COBSCServiceDlg, CDialogEx)

COBSCServiceDlg::COBSCServiceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LOG_OBC_SERVICE, pParent)
    , m_originalSize(0, 0)  // 初始化原始大小为0
{
}

COBSCServiceDlg::~COBSCServiceDlg()
{
}

BEGIN_MESSAGE_MAP(COBSCServiceDlg, CDialogEx)
    ON_WM_SIZE()
END_MESSAGE_MAP()

void COBSCServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT2_LOG_OBC_SERVICE, m_log_observer_client);
}

// 添加日志信息
void COBSCServiceDlg::AddLog(const CString& log_txt, COLORREF txt_color)
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

BOOL COBSCServiceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // 获取初始窗口大小
    CRect rect;
    GetClientRect(&rect);
    m_originalSize = rect.Size();

    // 初始化布局信息
    CWnd* pChild = GetWindow(GW_CHILD);
    while (pChild)
    {
        ControlLayoutInfo info;
        pChild->GetWindowRect(&info.originalRect);
        ScreenToClient(&info.originalRect);
        info.nID = pChild->GetDlgCtrlID();

        // 设置锚定方式
        if (info.nID == IDC_RICHEDIT2_LOG_OBC_SERVICE) {
            info.anchor = AnchorStyle::LEFT | AnchorStyle::TOP | AnchorStyle::RIGHT | AnchorStyle::BOTTOM;
        }
        else {
            info.anchor = AnchorStyle::LEFT | AnchorStyle::TOP;
        }

        m_layoutInfos.push_back(info);
        pChild = pChild->GetNextWindow();
    }

	// 初始化富文本编辑控件

	// 设置富文本编辑控件的属性
	//m_log_observer_client.SetWindowTextW(_T("[Event]21:35:22|处罚玩家:反外挂联盟传奇部 - 我是传奇 处罚类型:封机器 处罚原因:封机器码|178BFBFF00A50F00|00-50-56-C0-00-01|1142814342\r\n"));
	m_log_observer_client.SetBackgroundColor(false, RGB(0, 0, 0));
	//SetTextColor(RGB(0, 255, 0));	
	return TRUE;
}

void COBSCServiceDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (nType == SIZE_MINIMIZED || cx <= 0 || cy <= 0)
        return;

    // 计算大小变化量
    int deltaX = cx - m_originalSize.cx;
    int deltaY = cy - m_originalSize.cy;

    // 使用延迟窗口定位以获得更好的性能
    HDWP hdwp = BeginDeferWindowPos((int)m_layoutInfos.size());

    for (auto& info : m_layoutInfos)  // 修改为auto&以便更新原始位置
    {
        CWnd* pWnd = GetDlgItem(info.nID);
        if (!pWnd || !pWnd->GetSafeHwnd())
            continue;

        CRect newRect = info.originalRect;

        // 根据锚定方式调整位置和大小
        if ((info.anchor & AnchorStyle::RIGHT) != 0)
        {
            if ((info.anchor & AnchorStyle::LEFT) != 0)
            {
                // 左右都锚定，调整宽度
                newRect.right += deltaX;
            }
            else
            {
                // 只锚定右边，移动位置
                newRect.left += deltaX;
                newRect.right += deltaX;
            }
        }

        if ((info.anchor & AnchorStyle::BOTTOM) != 0)
        {
            if ((info.anchor & AnchorStyle::TOP) != 0)
            {
                // 上下都锚定，调整高度
                newRect.bottom += deltaY;
            }
            else
            {
                // 只锚定下边，移动位置
                newRect.top += deltaY;
                newRect.bottom += deltaY;
            }
        }

        hdwp = DeferWindowPos(hdwp, pWnd->m_hWnd, NULL,
                              newRect.left, newRect.top,
                              newRect.Width(), newRect.Height(),
                              SWP_NOZORDER);

        // 更新控件的原始位置信息
        info.originalRect = newRect;  // 添加这行以更新控件位置
    }

    if (hdwp)
        EndDeferWindowPos(hdwp);

    // 更新原始大小为当前大小
    m_originalSize = CSize(cx, cy);
}
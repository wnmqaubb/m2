// ScreenShotDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "ScreenShotDlg.h"
#include "afxdialogex.h"
#include <atlimage.h>


// ScreenShotDlg 对话框

IMPLEMENT_DYNAMIC(ScreenShotDlg, CDialogEx)

ScreenShotDlg::ScreenShotDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SCREENSHOT_DLG, pParent)
{

}

ScreenShotDlg::~ScreenShotDlg()
{
}

void ScreenShotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void ScreenShotDlg::ShowPicture()
{
	if(m_screenshot_path.IsEmpty()) return;
	float cx, cy, dx, dy, k, t;//跟控件的宽和高以及图片宽和高有关的参数
	CRect rect;//用于获取图片控件的宽和高
	CImage q;//为cimage图片类创建一个对象
	q.Load(m_screenshot_path);//构造函数的形参是所加载图片的路径 
	cx = (float)q.GetWidth();
	cy = (float)q.GetHeight();//获取图片的宽 高
	k = cy / cx;//获得图片的宽高比

	CWnd *pWnd = GetDlgItem(IDC_STATIC_SCREENSHOT);//获取控件句柄
	pWnd->UpdateWindow();
	pWnd->GetClientRect(&rect);//获取Picture Control控件的客户区
	dx = (float)rect.Width();
	dy = (float)rect.Height();//获得控件的宽高比
	t = dy / dx;//获得控件的宽高比
	if(k >= t)
	{
		rect.right = (long)floor(rect.bottom / k);
		rect.left = (long)(dx - rect.right) / 2;
		rect.right = long(floor(rect.bottom / k) + (dx - rect.right) / 2);
	} else
	{
		rect.bottom = (long)floor(k*rect.right);
		rect.top = (long)(dy - rect.bottom) / 2;
		rect.bottom = long(floor(k*rect.right) + (dy - rect.bottom) / 2);
	}
	//相关的计算为了让图片在绘图区居中按比例显示，原理很好懂，如果图片很宽但是不高，就上下留有空白区；如果图片很高而不宽就左右留有空白区，并且保持两边空白区一样大

	CDC *pDc = pWnd->GetDC();//获取picture control的DC，这是什么玩意我也不知道，百度就行
	int ModeOld = SetStretchBltMode(pDc->m_hDC, STRETCH_HALFTONE);//设置指定设备环境中的位图拉伸模式

	//COLORREF  colorVal = RGB(50, 50, 50);
	//pDc->FloodFill(0,0,colorVal);
	//pDc->FillRect(rect, &CBrush(colorVal));
	q.StretchBlt(pDc->m_hDC, rect, SRCCOPY);//显示函数
	SetStretchBltMode(pDc->m_hDC, ModeOld);
	ReleaseDC(pDc);//释放指针空间
}

BEGIN_MESSAGE_MAP(ScreenShotDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &ScreenShotDlg::OnBnClickedBrowseFile)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// ScreenShotDlg 消息处理程序

// 打开并定位文件
void ScreenShotDlg::OnBnClickedBrowseFile()
{
	// ShellExecute会被360报
	//ShellExecute(NULL, TEXT("open"), TEXT("explorer"), TEXT("/select, D:\\360Downloads\\test.bmp"), NULL, SW_SHOW);
	OpenFolderAndSelectFile(m_screenshot_path);
}

// 打开并定位文件
BOOL ScreenShotDlg::OpenFolderAndSelectFile(CString szPath)
{
	LPSHELLFOLDER pDesktopFolder;
	const wchar_t* screenshot_path = szPath.GetString();
	if(SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
		LPITEMIDLIST pidl;
		ULONG chEaten;
		ULONG dwAttributes;
		HRESULT hr;
		hr = pDesktopFolder->ParseDisplayName(
			NULL, NULL, (LPWSTR)screenshot_path, &chEaten, &pidl, &dwAttributes);
		if(FAILED(hr))
		{
			pDesktopFolder->Release();
			return FALSE;
		}
		LPCITEMIDLIST pidlFolder = pidl;
		CoInitialize(NULL);
		hr = SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
		pDesktopFolder->Release();
		if(hr == S_OK)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void ScreenShotDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
	GetDlgItem(IDC_STATIC_SCREENSHOT_PATH)->SetWindowText(m_screenshot_path);
	/*ShowPicture();*/
}

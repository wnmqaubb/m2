#include "pch.h"
#include "CScrollingText.h"


IMPLEMENT_DYNCREATE(CScrollingText, CStatic)

CScrollingText::CScrollingText()
	: m_nOffset(0), m_bScrolling(false)
{
	// 创建默认字体
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	_tcscpy_s(lf.lfFaceName, _T("新宋体"));
	lf.lfHeight = -MulDiv(16, ::GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72); // 24 pt 字体
	lf.lfWeight = FW_BOLD; // 加粗字体
	m_font.CreateFontIndirect(&lf);
}

CScrollingText::~CScrollingText()
{
	StopScrolling();
}

void CScrollingText::SetText(const CString& str)
{
	m_strText = str;
	Invalidate(); // 使窗口无效以触发重绘
}
BEGIN_MESSAGE_MAP(CScrollingText, CStatic)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()
void CScrollingText::StartScrolling()
{
	if (!m_bScrolling)
	{
		m_bScrolling = true;
		SetTimer(1, SCROLL_INTERVAL, nullptr);
	}
}

void CScrollingText::StopScrolling()
{
	if (m_bScrolling)
	{
		KillTimer(1);
		m_bScrolling = false;
		m_nOffset = 0;
		Invalidate();
	}
}
void CScrollingText::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		CClientDC dc(this);
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);

		CRect rect;
		GetClientRect(&rect);

		// 计算文本宽度
		CFont* pOldFont = memDC.SelectObject(&m_font);
		CSize textSize = dc.GetTextExtent(m_strText);

		// 更新偏移量
		m_nOffset -= SCROLL_SPEED;

		// 如果文本已经完全滚出视图，则重置偏移量
		if (m_nOffset <= -textSize.cx)
		{
			m_nOffset = rect.Width();
		}

		// 重绘
		Invalidate();
		UpdateWindow();

		memDC.SelectObject(pOldFont);
	}

	CStatic::OnTimer(nIDEvent);
}

void CScrollingText::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);

	// 清除背景
	dc.FillSolidRect(&rect, RGB(0, 0, 0)); // 白色背景
	// 设置随机颜色
	COLORREF color = RGB(255, 30, 30);
	dc.SetTextColor(color);
	// 计算文本宽度
	CFont* pOldFont = dc.SelectObject(&m_font);
	CSize textSize = dc.GetTextExtent(m_strText);

	// 设置剪切区域
	CRgn clipRgn;
	clipRgn.CreateRectRgn(0, 0, rect.Width(), rect.Height());
	dc.SelectClipRgn(&clipRgn);
	int nTop = (rect.Height() - textSize.cy) / 2;
	// 绘制文本
	dc.TextOut(rect.left + m_nOffset, nTop, m_strText);

	// 释放剪切区域
	dc.SelectClipRgn(nullptr);
	clipRgn.DeleteObject();
}

void CScrollingText::SetFont(LOGFONT& lf)
{
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf);
	Invalidate();
}
#include "pch.h"
#include "CScrollingText.h"


IMPLEMENT_DYNCREATE(CScrollingText, CStatic)

CScrollingText::CScrollingText()
	: m_nOffset(0), m_bScrolling(false)
{
	// ����Ĭ������
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	_tcscpy_s(lf.lfFaceName, _T("������"));
	lf.lfHeight = -MulDiv(16, ::GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72); // 24 pt ����
	lf.lfWeight = FW_BOLD; // �Ӵ�����
	m_font.CreateFontIndirect(&lf);
}

CScrollingText::~CScrollingText()
{
	StopScrolling();
}

void CScrollingText::SetText(const CString& str)
{
	m_strText = str;
	Invalidate(); // ʹ������Ч�Դ����ػ�
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

		// �����ı����
		CFont* pOldFont = memDC.SelectObject(&m_font);
		CSize textSize = dc.GetTextExtent(m_strText);

		// ����ƫ����
		m_nOffset -= SCROLL_SPEED;

		// ����ı��Ѿ���ȫ������ͼ��������ƫ����
		if (m_nOffset <= -textSize.cx)
		{
			m_nOffset = rect.Width();
		}

		// �ػ�
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

	// �������
	dc.FillSolidRect(&rect, RGB(0, 0, 0)); // ��ɫ����
	// ���������ɫ
	COLORREF color = RGB(255, 30, 30);
	dc.SetTextColor(color);
	// �����ı����
	CFont* pOldFont = dc.SelectObject(&m_font);
	CSize textSize = dc.GetTextExtent(m_strText);

	// ���ü�������
	CRgn clipRgn;
	clipRgn.CreateRectRgn(0, 0, rect.Width(), rect.Height());
	dc.SelectClipRgn(&clipRgn);
	int nTop = (rect.Height() - textSize.cy) / 2;
	// �����ı�
	dc.TextOut(rect.left + m_nOffset, nTop, m_strText);

	// �ͷż�������
	dc.SelectClipRgn(nullptr);
	clipRgn.DeleteObject();
}

void CScrollingText::SetFont(LOGFONT& lf)
{
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf);
	Invalidate();
}
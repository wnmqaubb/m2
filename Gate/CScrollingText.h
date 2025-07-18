#pragma once

class CScrollingText : public CStatic
{
	DECLARE_DYNCREATE(CScrollingText)
public:
	CScrollingText();
	virtual ~CScrollingText();
	using CStatic::Create;
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	void SetFont(LOGFONT& lf);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	CString m_strText;
	int m_nOffset;
	bool m_bScrolling;
	const int SCROLL_SPEED = 3; // �����ٶ�
	const int SCROLL_INTERVAL = 100; // ������������룩
	CFont m_font; // �Զ�������

public:
	void SetText(const CString& str);
	void StartScrolling();
	void StopScrolling();
};
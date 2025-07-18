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
	const int SCROLL_SPEED = 3; // 滚动速度
	const int SCROLL_INTERVAL = 100; // 滚动间隔（毫秒）
	CFont m_font; // 自定义字体

public:
	void SetText(const CString& str);
	void StartScrolling();
	void StopScrolling();
};
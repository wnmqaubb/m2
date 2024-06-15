<<<<<<< HEAD
﻿// generaldlg.cpp: 实现文件
//

#include "pch.h"
#include "mircheat.h"
#include "generaldlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "cheat_global.h"


// GeneralDlg 对话框

IMPLEMENT_DYNAMIC(GeneralDlg, CDialogEx)

GeneralDlg::GeneralDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN_GENERAL_DLG, pParent)
    , m_general_move_speed_static(427)
    , m_general_attack_speed_static(451)
    , m_general_magic_speed_static(427)
    , m_general_move_interval_static(427)
    , m_general_attack_interval_static(427)
    , m_general_magic_interval_static(427)
    , m_general_speed_change_static(100)
    , m_general_speed_change_static_str(L"1.00")
{

}

GeneralDlg::~GeneralDlg()
{
}

void GeneralDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER1, m_general_move_speed_slider);
    DDX_Control(pDX, IDC_SLIDER2, m_general_attack_speed_slider);
    DDX_Control(pDX, IDC_SLIDER3, m_general_magic_speed_slider);
    DDX_Control(pDX, IDC_SLIDER4, m_general_move_interval_slider);
    DDX_Control(pDX, IDC_SLIDER5, m_general_attack_interval_slider);
    DDX_Control(pDX, IDC_SLIDER6, m_general_magic_interval_slider);
    DDX_Control(pDX, IDC_SLIDER7, m_general_speed_change_slider);
    DDX_Text(pDX, IDC_GENERAL_MOVE_SPEED_STATIC, m_general_move_speed_static);
    DDX_Text(pDX, IDC_GENERAL_ATTACK_SPEED_STATIC, m_general_attack_speed_static);
    DDX_Text(pDX, IDC_GENERAL_MAGIC_SPEED_STATIC, m_general_magic_speed_static);
    DDX_Text(pDX, IDC_GENERAL_MOVE_INTERVAL_STATIC, m_general_move_interval_static);
    DDX_Text(pDX, IDC_GENERAL_ATTACK_INTERVAL_STATIC, m_general_attack_interval_static);
    DDX_Text(pDX, IDC_GENERAL_MAGIC_INTERVAL_STATIC, m_general_magic_interval_static);
    DDX_Text(pDX, IDC_GENERAL_SPEED_CHANGE_STATIC, m_general_speed_change_static_str);
}


BEGIN_MESSAGE_MAP(GeneralDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON1, &GeneralDlg::OnBnClickedGeneralMoveSpeed)
    ON_WM_HSCROLL()
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &GeneralDlg::OnDeltaposGeneralMoveSpeed)
    ON_BN_CLICKED(IDC_BUTTON2, &GeneralDlg::OnBnClickedGeneralAttackSpeed)
    ON_BN_CLICKED(IDC_BUTTON3, &GeneralDlg::OnBnClickedGeneralMagicSpeed)
    ON_BN_CLICKED(IDC_BUTTON4, &GeneralDlg::OnBnClickedGeneralMoveInterval)
    ON_BN_CLICKED(IDC_BUTTON5, &GeneralDlg::OnBnClickedGeneralAttackInterval)
    ON_BN_CLICKED(IDC_BUTTON6, &GeneralDlg::OnBnClickedGeneralMagicInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &GeneralDlg::OnDeltaposGeneralAttackSpeed)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, &GeneralDlg::OnDeltaposGeneralMagicSpeed)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, &GeneralDlg::OnDeltaposGeneralMoveInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, &GeneralDlg::OnDeltaposGeneralAttackInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN6, &GeneralDlg::OnDeltaposGeneralMagicInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN7, &GeneralDlg::OnDeltaposGeneralSpeedChange)
    ON_BN_CLICKED(IDC_CHECK9, &GeneralDlg::OnBnClickedCheck9)
    ON_BN_CLICKED(IDC_CHECK1, &GeneralDlg::OnBnClickedCheck1)
END_MESSAGE_MAP()


// GeneralDlg 消息处理程序


BOOL GeneralDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_general_move_speed_slider.SetRange(0, 500);
    m_general_move_speed_slider.SetTicFreq(1);
    m_general_move_speed_slider.SetPos(427);

    m_general_attack_speed_slider.SetRange(0, 500);
    m_general_attack_speed_slider.SetTicFreq(1);
    m_general_attack_speed_slider.SetPos(451);

    m_general_magic_speed_slider.SetRange(0, 500);
    m_general_magic_speed_slider.SetTicFreq(1);
    m_general_magic_speed_slider.SetPos(427);

    m_general_move_interval_slider.SetRange(0, 1400);
    m_general_move_interval_slider.SetTicFreq(1);
    m_general_move_interval_slider.SetPos(427);

    m_general_attack_interval_slider.SetRange(0, 1400);
    m_general_attack_interval_slider.SetTicFreq(1);
    m_general_attack_interval_slider.SetPos(427);

    m_general_magic_interval_slider.SetRange(0, 1400);
    m_general_magic_interval_slider.SetTicFreq(1);
    m_general_magic_interval_slider.SetPos(427);

    m_general_speed_change_slider.SetRange(100, 500);
    m_general_speed_change_slider.SetTicFreq(1);
    m_general_speed_change_slider.SetPos(1);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}



void GeneralDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CSliderCtrl* EventSliderCtrl = (CSliderCtrl*)pScrollBar;
    if(&m_general_move_speed_slider == EventSliderCtrl)
    {
        m_general_move_speed_static = m_general_move_speed_slider.GetPos();
        general_move_speed = 500 - m_general_move_speed_static;
        UpdateData(FALSE);
    }
    else if(&m_general_attack_speed_slider == EventSliderCtrl)
    {
        m_general_attack_speed_static = m_general_attack_speed_slider.GetPos();
        general_attack_speed = 500 - m_general_attack_speed_static;
        UpdateData(FALSE);
    }
    else if(&m_general_magic_speed_slider == EventSliderCtrl)
    {
        m_general_magic_speed_static = m_general_magic_speed_slider.GetPos();
        general_magic_speed = 500 - m_general_magic_speed_static;
        UpdateData(FALSE);
    }
    else if(&m_general_move_interval_slider == EventSliderCtrl)
    {
        m_general_move_interval_static = m_general_move_interval_slider.GetPos();
        general_move_interval = 1400 - m_general_move_interval_static;
        UpdateData(FALSE);
    }
    else if(&m_general_attack_interval_slider == EventSliderCtrl)
    {
        m_general_attack_interval_static = m_general_attack_interval_slider.GetPos();
        general_attack_interval = 1400 - m_general_attack_interval_static;
        UpdateData(FALSE);
    }
    else if(&m_general_magic_interval_slider == EventSliderCtrl)
    {
        m_general_magic_interval_static = m_general_magic_interval_slider.GetPos();
        general_magic_interval = 1400 - m_general_magic_interval_static;
        UpdateData(FALSE);
    }
    else if(&m_general_speed_change_slider == EventSliderCtrl)
    {
        m_general_speed_change_static = m_general_speed_change_slider.GetPos();
        acceleration = m_general_speed_change_static / 100.0;
        m_general_speed_change_static_str.Format(L"%.2f", acceleration);
        UpdateData(FALSE);
    }

    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void GeneralDlg::OnBnClickedGeneralMoveSpeed()
{
    m_general_move_speed_slider.SetPos(427);
    m_general_move_speed_static = 427;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralAttackSpeed()
{
    m_general_attack_speed_slider.SetPos(451);
    m_general_attack_speed_static = 451;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralMagicSpeed()
{
    m_general_magic_speed_slider.SetPos(427);
    m_general_magic_speed_static = 427;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralMoveInterval()
{
    m_general_move_interval_slider.SetPos(1400);
    m_general_move_interval_static = 1400;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralAttackInterval()
{
    m_general_attack_interval_slider.SetPos(1400);
    m_general_attack_interval_static = 1400;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralMagicInterval()
{
    m_general_magic_interval_slider.SetPos(1400);
    m_general_magic_interval_static = 1400;
    UpdateData(FALSE);
}


void GeneralDlg::OnDeltaposGeneralMoveSpeed(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_move_speed_static > 0) || (pNMUpDown->iDelta < 0 && m_general_move_speed_static < 500))
    {
        m_general_move_speed_static -= pNMUpDown->iDelta;
    }
    m_general_move_speed_slider.SetPos(m_general_move_speed_static);
    general_move_speed = 500 - m_general_move_speed_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralAttackSpeed(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_attack_speed_static > 0) || (pNMUpDown->iDelta < 0 && m_general_attack_speed_static < 500))
    {
        m_general_attack_speed_static -= pNMUpDown->iDelta;
    }
    m_general_attack_speed_slider.SetPos(m_general_attack_speed_static);
    general_attack_speed = 500 - m_general_attack_speed_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralMagicSpeed(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_magic_speed_static > 0) || (pNMUpDown->iDelta < 0 && m_general_magic_speed_static < 500))
    {
        m_general_magic_speed_static -= pNMUpDown->iDelta;
    }
    m_general_magic_speed_slider.SetPos(m_general_magic_speed_static);
    general_magic_speed = 500 - m_general_magic_speed_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralMoveInterval(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_move_interval_static > 0) || (pNMUpDown->iDelta < 0 && m_general_move_interval_static < 1400))
    {
        m_general_move_interval_static -= pNMUpDown->iDelta;
    }
    m_general_move_interval_slider.SetPos(m_general_move_interval_static);
    general_move_interval = 1400 - m_general_move_interval_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralAttackInterval(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_attack_interval_static > 0) || (pNMUpDown->iDelta < 0 && m_general_attack_interval_static < 1400))
    {
        m_general_attack_interval_static -= pNMUpDown->iDelta;
    }
    m_general_attack_interval_slider.SetPos(m_general_attack_interval_static);
    general_attack_interval = 1400 - m_general_attack_interval_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralMagicInterval(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_magic_interval_static > 0) || (pNMUpDown->iDelta < 0 && m_general_magic_interval_static < 1400))
    {
        m_general_magic_interval_static -= pNMUpDown->iDelta;
    }
    m_general_magic_interval_slider.SetPos(m_general_magic_interval_static);
    general_magic_interval = 1400 - m_general_magic_interval_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralSpeedChange(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    m_general_speed_change_static = m_general_speed_change_slider.GetPos();
    if((pNMUpDown->iDelta > 0 && m_general_speed_change_static > 100) || (pNMUpDown->iDelta < 0 && m_general_speed_change_static < 500))
    {
        m_general_speed_change_static -= pNMUpDown->iDelta;
    }
    acceleration = m_general_speed_change_static / 100.0;
    m_general_speed_change_slider.SetPos(m_general_speed_change_static);
    m_general_speed_change_static_str.Format(L"%.2f", acceleration);
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnBnClickedCheck9()
{
    general_speed_checkbox = ((CButton*)GetDlgItem(IDC_CHECK9))->GetCheck();
}


void GeneralDlg::OnBnClickedCheck1()
{
    general_speed_change_checkbox = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
=======
﻿// generaldlg.cpp: 实现文件
//

#include "pch.h"
#include "mircheat.h"
#include "generaldlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "cheat_global.h"


// GeneralDlg 对话框

IMPLEMENT_DYNAMIC(GeneralDlg, CDialogEx)

GeneralDlg::GeneralDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN_GENERAL_DLG, pParent)
    , m_general_move_speed_static(427)
    , m_general_attack_speed_static(451)
    , m_general_magic_speed_static(427)
    , m_general_move_interval_static(427)
    , m_general_attack_interval_static(427)
    , m_general_magic_interval_static(427)
    , m_general_speed_change_static(100)
    , m_general_speed_change_static_str(L"1.00")
{

}

GeneralDlg::~GeneralDlg()
{
}

void GeneralDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER1, m_general_move_speed_slider);
    DDX_Control(pDX, IDC_SLIDER2, m_general_attack_speed_slider);
    DDX_Control(pDX, IDC_SLIDER3, m_general_magic_speed_slider);
    DDX_Control(pDX, IDC_SLIDER4, m_general_move_interval_slider);
    DDX_Control(pDX, IDC_SLIDER5, m_general_attack_interval_slider);
    DDX_Control(pDX, IDC_SLIDER6, m_general_magic_interval_slider);
    DDX_Control(pDX, IDC_SLIDER7, m_general_speed_change_slider);
    DDX_Text(pDX, IDC_GENERAL_MOVE_SPEED_STATIC, m_general_move_speed_static);
    DDX_Text(pDX, IDC_GENERAL_ATTACK_SPEED_STATIC, m_general_attack_speed_static);
    DDX_Text(pDX, IDC_GENERAL_MAGIC_SPEED_STATIC, m_general_magic_speed_static);
    DDX_Text(pDX, IDC_GENERAL_MOVE_INTERVAL_STATIC, m_general_move_interval_static);
    DDX_Text(pDX, IDC_GENERAL_ATTACK_INTERVAL_STATIC, m_general_attack_interval_static);
    DDX_Text(pDX, IDC_GENERAL_MAGIC_INTERVAL_STATIC, m_general_magic_interval_static);
    DDX_Text(pDX, IDC_GENERAL_SPEED_CHANGE_STATIC, m_general_speed_change_static_str);
}


BEGIN_MESSAGE_MAP(GeneralDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON1, &GeneralDlg::OnBnClickedGeneralMoveSpeed)
    ON_WM_HSCROLL()
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &GeneralDlg::OnDeltaposGeneralMoveSpeed)
    ON_BN_CLICKED(IDC_BUTTON2, &GeneralDlg::OnBnClickedGeneralAttackSpeed)
    ON_BN_CLICKED(IDC_BUTTON3, &GeneralDlg::OnBnClickedGeneralMagicSpeed)
    ON_BN_CLICKED(IDC_BUTTON4, &GeneralDlg::OnBnClickedGeneralMoveInterval)
    ON_BN_CLICKED(IDC_BUTTON5, &GeneralDlg::OnBnClickedGeneralAttackInterval)
    ON_BN_CLICKED(IDC_BUTTON6, &GeneralDlg::OnBnClickedGeneralMagicInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &GeneralDlg::OnDeltaposGeneralAttackSpeed)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, &GeneralDlg::OnDeltaposGeneralMagicSpeed)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, &GeneralDlg::OnDeltaposGeneralMoveInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, &GeneralDlg::OnDeltaposGeneralAttackInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN6, &GeneralDlg::OnDeltaposGeneralMagicInterval)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN7, &GeneralDlg::OnDeltaposGeneralSpeedChange)
    ON_BN_CLICKED(IDC_CHECK9, &GeneralDlg::OnBnClickedCheck9)
    ON_BN_CLICKED(IDC_CHECK1, &GeneralDlg::OnBnClickedCheck1)
END_MESSAGE_MAP()


// GeneralDlg 消息处理程序


BOOL GeneralDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_general_move_speed_slider.SetRange(0, 500);
    m_general_move_speed_slider.SetTicFreq(1);
    m_general_move_speed_slider.SetPos(427);

    m_general_attack_speed_slider.SetRange(0, 500);
    m_general_attack_speed_slider.SetTicFreq(1);
    m_general_attack_speed_slider.SetPos(451);

    m_general_magic_speed_slider.SetRange(0, 500);
    m_general_magic_speed_slider.SetTicFreq(1);
    m_general_magic_speed_slider.SetPos(427);

    m_general_move_interval_slider.SetRange(0, 1400);
    m_general_move_interval_slider.SetTicFreq(1);
    m_general_move_interval_slider.SetPos(427);

    m_general_attack_interval_slider.SetRange(0, 1400);
    m_general_attack_interval_slider.SetTicFreq(1);
    m_general_attack_interval_slider.SetPos(427);

    m_general_magic_interval_slider.SetRange(0, 1400);
    m_general_magic_interval_slider.SetTicFreq(1);
    m_general_magic_interval_slider.SetPos(427);

    m_general_speed_change_slider.SetRange(100, 500);
    m_general_speed_change_slider.SetTicFreq(1);
    m_general_speed_change_slider.SetPos(1);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}



void GeneralDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CSliderCtrl* EventSliderCtrl = (CSliderCtrl*)pScrollBar;
    if(&m_general_move_speed_slider == EventSliderCtrl)
    {
        m_general_move_speed_static = m_general_move_speed_slider.GetPos();
        general_move_speed = 500 - m_general_move_speed_static;
        UpdateData(FALSE);
    }
    else if(&m_general_attack_speed_slider == EventSliderCtrl)
    {
        m_general_attack_speed_static = m_general_attack_speed_slider.GetPos();
        general_attack_speed = 500 - m_general_attack_speed_static;
        UpdateData(FALSE);
    }
    else if(&m_general_magic_speed_slider == EventSliderCtrl)
    {
        m_general_magic_speed_static = m_general_magic_speed_slider.GetPos();
        general_magic_speed = 500 - m_general_magic_speed_static;
        UpdateData(FALSE);
    }
    else if(&m_general_move_interval_slider == EventSliderCtrl)
    {
        m_general_move_interval_static = m_general_move_interval_slider.GetPos();
        general_move_interval = 1400 - m_general_move_interval_static;
        UpdateData(FALSE);
    }
    else if(&m_general_attack_interval_slider == EventSliderCtrl)
    {
        m_general_attack_interval_static = m_general_attack_interval_slider.GetPos();
        general_attack_interval = 1400 - m_general_attack_interval_static;
        UpdateData(FALSE);
    }
    else if(&m_general_magic_interval_slider == EventSliderCtrl)
    {
        m_general_magic_interval_static = m_general_magic_interval_slider.GetPos();
        general_magic_interval = 1400 - m_general_magic_interval_static;
        UpdateData(FALSE);
    }
    else if(&m_general_speed_change_slider == EventSliderCtrl)
    {
        m_general_speed_change_static = m_general_speed_change_slider.GetPos();
        acceleration = m_general_speed_change_static / 100.0;
        m_general_speed_change_static_str.Format(L"%.2f", acceleration);
        UpdateData(FALSE);
    }

    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void GeneralDlg::OnBnClickedGeneralMoveSpeed()
{
    m_general_move_speed_slider.SetPos(427);
    m_general_move_speed_static = 427;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralAttackSpeed()
{
    m_general_attack_speed_slider.SetPos(451);
    m_general_attack_speed_static = 451;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralMagicSpeed()
{
    m_general_magic_speed_slider.SetPos(427);
    m_general_magic_speed_static = 427;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralMoveInterval()
{
    m_general_move_interval_slider.SetPos(1400);
    m_general_move_interval_static = 1400;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralAttackInterval()
{
    m_general_attack_interval_slider.SetPos(1400);
    m_general_attack_interval_static = 1400;
    UpdateData(FALSE);
}


void GeneralDlg::OnBnClickedGeneralMagicInterval()
{
    m_general_magic_interval_slider.SetPos(1400);
    m_general_magic_interval_static = 1400;
    UpdateData(FALSE);
}


void GeneralDlg::OnDeltaposGeneralMoveSpeed(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_move_speed_static > 0) || (pNMUpDown->iDelta < 0 && m_general_move_speed_static < 500))
    {
        m_general_move_speed_static -= pNMUpDown->iDelta;
    }
    m_general_move_speed_slider.SetPos(m_general_move_speed_static);
    general_move_speed = 500 - m_general_move_speed_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralAttackSpeed(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_attack_speed_static > 0) || (pNMUpDown->iDelta < 0 && m_general_attack_speed_static < 500))
    {
        m_general_attack_speed_static -= pNMUpDown->iDelta;
    }
    m_general_attack_speed_slider.SetPos(m_general_attack_speed_static);
    general_attack_speed = 500 - m_general_attack_speed_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralMagicSpeed(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_magic_speed_static > 0) || (pNMUpDown->iDelta < 0 && m_general_magic_speed_static < 500))
    {
        m_general_magic_speed_static -= pNMUpDown->iDelta;
    }
    m_general_magic_speed_slider.SetPos(m_general_magic_speed_static);
    general_magic_speed = 500 - m_general_magic_speed_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralMoveInterval(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_move_interval_static > 0) || (pNMUpDown->iDelta < 0 && m_general_move_interval_static < 1400))
    {
        m_general_move_interval_static -= pNMUpDown->iDelta;
    }
    m_general_move_interval_slider.SetPos(m_general_move_interval_static);
    general_move_interval = 1400 - m_general_move_interval_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralAttackInterval(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_attack_interval_static > 0) || (pNMUpDown->iDelta < 0 && m_general_attack_interval_static < 1400))
    {
        m_general_attack_interval_static -= pNMUpDown->iDelta;
    }
    m_general_attack_interval_slider.SetPos(m_general_attack_interval_static);
    general_attack_interval = 1400 - m_general_attack_interval_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralMagicInterval(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    if((pNMUpDown->iDelta > 0 && m_general_magic_interval_static > 0) || (pNMUpDown->iDelta < 0 && m_general_magic_interval_static < 1400))
    {
        m_general_magic_interval_static -= pNMUpDown->iDelta;
    }
    m_general_magic_interval_slider.SetPos(m_general_magic_interval_static);
    general_magic_interval = 1400 - m_general_magic_interval_static;
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnDeltaposGeneralSpeedChange(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    m_general_speed_change_static = m_general_speed_change_slider.GetPos();
    if((pNMUpDown->iDelta > 0 && m_general_speed_change_static > 100) || (pNMUpDown->iDelta < 0 && m_general_speed_change_static < 500))
    {
        m_general_speed_change_static -= pNMUpDown->iDelta;
    }
    acceleration = m_general_speed_change_static / 100.0;
    m_general_speed_change_slider.SetPos(m_general_speed_change_static);
    m_general_speed_change_static_str.Format(L"%.2f", acceleration);
    UpdateData(FALSE);
    *pResult = 0;
}


void GeneralDlg::OnBnClickedCheck9()
{
    general_speed_checkbox = ((CButton*)GetDlgItem(IDC_CHECK9))->GetCheck();
}


void GeneralDlg::OnBnClickedCheck1()
{
    general_speed_change_checkbox = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
}
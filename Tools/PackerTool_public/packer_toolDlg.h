// packer_toolDlg.h: 头文件

#pragma once

#include <afxcmn.h>
#include <afxdialogex.h>
#include <afxstr.h>
#include <afxwin.h>
#include <filesystem>
#include <mutex>
#include <shellapi.h>
#include <string>
#include <vector>
#include <Windows.h>

// CpackertoolDlg 对话框
class CpackertoolDlg : public CDialogEx
{
// 构造
public:
    CpackertoolDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_PACKER_TOOL_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    bool exec_cmd(CString cmd, bool show_log = true);
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

private:
    std::vector<CString> m_draggedFiles;  // 存储拖动的文件列表
    std::mutex m_mutex;
    volatile LONG m_completedTasks; // 使用原子操作
    volatile LONG m_totalTasks;

    void PackFileThread(const std::filesystem::path& pack_exe_path);  // 线程打包函数

    CString packer_tool_validate(const std::string& snhash);

public:
    afx_msg void OnBnClickedButtonPack();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void SHowLog(CString log_text = L"");
    afx_msg void OnBnClickedVMP(); 
    LRESULT OnUpdateWindow(WPARAM wParam, LPARAM lParam);
    void OnOpenMultipleFiles();
    void MyUpdateWindow();
    CEdit m_pack_file_edit;
    CEdit m_result_edit;
    CProgressCtrl m_progress_ctrl;
    afx_msg void OnBnClickedValidateSnhash();
private:
    CString m_snhash;
    CString m_ip;
public:
    CString m_login_stauts;
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};

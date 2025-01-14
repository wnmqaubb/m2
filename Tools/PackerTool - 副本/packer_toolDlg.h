// packer_toolDlg.h: 头文件

#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "resource.h"

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
    bool exec_cmd(CString cmd);
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

private:
    std::vector<CString> m_draggedFiles;  // 存储拖动的文件列表
    std::mutex m_mutex;
    std::atomic<int> m_completedTasks{0};
    std::atomic<int> m_totalTasks{0};

    void PackFileThread(const CString& inputFile);  // 线程打包函数
    LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);  // 进度更新消息处理

public:
    afx_msg void OnBnClickedButtonPack();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnBnClickedLog();
    afx_msg void OnBnClickedVMP();
    void OnBnClickedButton2();
    CEdit m_pack_file_edit;
    CEdit m_result_edit;
};

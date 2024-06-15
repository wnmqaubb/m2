
// agentDlg.h: 头文件
//

#pragma once
#include "HPTypeDef.h"
#include "protocol.h"
#include "ProcessInfoDlg.h"
#include "ScreenShotDlg.h"
#include "ClientWindowDlg.h"
#include "ruleDlg.h"
#include "ShellManagerDlg.h"
#include "LogDlg.h"
#include <set>

typedef struct _GamerInfo
{
	int dwConnID;
	std::wstring userid;
	std::wstring username;
	std::wstring pack_ip;
	std::wstring machineID;
}GamerInfo,*PGamerInfo;

// CagentDlg 对话框
class CagentDlg : public CDialogEx, CTcpServerListener, TaskMgr
{
	// 构造
public:
	CagentDlg(CWnd* pParent = nullptr);	// 标准构造函数
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_AGENT_DIALOG
	};
#endif
	typedef enum _DECTECTION_TYPE
	{
		MULTITUDE_RUN = 1,
		MACHINEID = 2
	}DECTECTION_TYPE;
	typedef enum _SETTIMEOUT_ID
	{
		RELOAD_GAMER_LIST = 1,
		VMP_SERIAL_VALIDATE,
        ONLINE_CHECK,
        MACHINE_BLACK_LIST,
        SHELL_CODE_ACTION_COUNT, 
        IP_TABLES_CLEAR
	}SETTIMEOUT_ID;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 选中的记录的connect_id
	CONNID m_current_connect_id;
	int m_current_row_index;
    uint32_t m_start_tickcount;
	CString m_search_text;
    CString m_search_text_old;
	CTcpPackServerPtr m_server;
	bool m_server_started;
	CListCtrl m_client_list;
	ProcessInfoDlg m_processInfoDlg;
	ScreenShotDlg m_screenshotdlg;
	ClientWindowDlg m_clientWindowDlg;
	ShellManagerDlg m_shellManagerDlg;
	RuleDlg m_ruleDlg;
    LogDlg m_LogDlg;
	MFCUtil mfcutil;
	// 检测出用外挂的玩家的connid
	std::set<int> m_suspicious_connids;
    std::map<int, PGamerInfo> m_gamer_list;
	//CMap<int, int, GamerInfo, GamerInfo> m_gamer_list;
	afx_msg void OnBnClickedListen();
    std::tuple<std::wstring, int> GetClientAddress(ITcpServer * pSender, CONNID dwConnID);
	//afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedScreenshot();
	afx_msg void OnBnClickedShowRuleDlg();
	afx_msg void OnBnClickedBSOD();
	afx_msg void OnBnClickedGetProcesses();
	afx_msg void OnBnClickedGetWindows();
	afx_msg void OnBnClickedSendShellCode();
    void MachineBlackListCheck();
    void OnlineCheck();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMRClickListRightMenu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg void OnNMClickListRow(NMHDR *pNMHDR, LRESULT *pResult);
    void MultiClientDetect();
    afx_msg void OnBnClickedDetectionCheat();
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient);
	virtual EnHandleResult OnHandShake(ITcpServer* pSender, CONNID dwConnID);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
	void EnumConnectID(std::function<void(CONNID)> callback);
	void ReloadGamerList();
	void SendToAll(unsigned char* buffer, size_t buffer_size);
    void SendShellCode(uintptr_t conn_id, Protocol & package);
    void SendToAll(Protocol& proto);
    void Send(uintptr_t connid, Protocol& proto);
	void FlushGamerList(uintptr_t conn_id, const ProtocolHeartBeat& data);
	void ShowProcessInfo(ProtocolProcessInfo& data);
	void ShowScreenshotDlg();
	void ShowClientWindowDlg(ProtocolProcessInfo& data);
	CString GetConnectInfo(CONNID dwConnID);
	CONNID GetSelectedConnID();
protected:
	afx_msg LRESULT OnAsyncDoModal(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnLvnColumnclickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuOffline();
	afx_msg void OnBnClickedShellManager();
	afx_msg void OnCopyMachineid();
	afx_msg void OnAddBlacklist();
    afx_msg void OnBnClickedSwitchLogDlg();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    std::string ReadVmpSerialFile(std::filesystem::path & txtPath);
	bool VmpSerialValidate();
    bool VmpSerialValidate1(std::string & serial);
	int SetSocketRecvTimeout(int sockfd, uint64_t iTimeoutMs);
	CStringA HttpPost(CStringA strIP, int nPort, CStringA strUrl, CStringA param);
	void GetCurrentHWID();
	afx_msg void OnBnClickedButtonOnlineGamerSearch();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
    BOOL TrayMyIcon(BOOL bAdd);//bAdd为TRUE就添加，bAdd为FALSE就不添加。
    LRESULT OnTrayCallBackMsg(WPARAM wparam, LPARAM lparam);
    int m_back_game_lazy_time;
    int m_exit_game_lazy_time; 
    const int kMaxIPConnectCount = 100;
    std::mutex ip_tables_mtx_;
    afx_msg void OnEnChangeEdit1();
    bool OnGetShellCodeHeartBeatFlag();
    void ShowShellCodeCount();
    afx_msg void OnEnChangeEdit3();
    afx_msg void OnBnClickedBackGameLazyEnable();
    afx_msg void OnBnClickedExitGameLazyEnable();
};


#define ASYNC_DO_MODAL WM_USER+1
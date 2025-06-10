#pragma once

#include "ClientViewList.h"
#include "SearchBar.h"
#include "MainBar.h"
#include "Service/SubServicePackage.h"
#include <asio2/util/uuid.hpp>
#define PAGE_SIZE 1000  // 每页显示1000条

struct UserData {
    // 基本类型成员
    unsigned int session_id;
    std::wstring username;
    std::string ip;
    std::wstring cpuid;
    std::wstring mac;
    std::wstring vol;
    int sysver;
    int is_64bits;
    std::string commit_ver;
    unsigned int pid;
    time_t logintime;
    int miss_count;
    std::string client_address;
    std::string client_port;

    // 需要特殊处理的成员
    asio2::uuid uuid;

    // 默认构造函数
    UserData() = default;

    // 移动构造函数
    UserData(UserData&& other) noexcept
        : session_id(other.session_id),
        username(std::move(other.username)),
        ip(std::move(other.ip)),
        cpuid(std::move(other.cpuid)),
        mac(std::move(other.mac)),
        vol(std::move(other.vol)),
        sysver(other.sysver),
        is_64bits(other.is_64bits),
        commit_ver(std::move(other.commit_ver)),
        pid(other.pid),
        logintime(other.logintime),
        miss_count(other.miss_count),
        client_address(std::move(other.client_address)),
        client_port(std::move(other.client_port))
    {
        memcpy(uuid.data, other.uuid.data, sizeof(uuid.data));
    }

    // 禁用拷贝构造函数
    UserData(const UserData&) = delete;
    UserData& operator=(const UserData&) = delete;

    // 移动赋值运算符
    UserData& operator=(UserData&& other) noexcept {
        if (this != &other) {
            session_id = other.session_id;
            username = std::move(other.username);
            ip = std::move(other.ip);
            cpuid = std::move(other.cpuid);
            mac = std::move(other.mac);
            vol = std::move(other.vol);
            sysver = other.sysver;
            is_64bits = other.is_64bits;
            commit_ver = std::move(other.commit_ver);
            pid = other.pid;
            logintime = other.logintime;
            miss_count = other.miss_count;
            client_address = std::move(other.client_address);
            client_port = std::move(other.client_port);
            memcpy(uuid.data, other.uuid.data, sizeof(uuid.data));
        }
        return *this;
    }

    std::string get_uuid_str() const {
        char buf[37] = { 0 };
        asio2::uuid u;
        memcpy(u.data, uuid.data, sizeof(u.data));
        return u.str(buf);
    }
};

class CClientView : public CDockablePane
{
public:
	CClientView() noexcept;
	virtual ~CClientView();

	void AdjustLayout();
	void OnChangeVisualStyle();
    CString GetCurrentSelectedUserName();

    template<typename T> void SendCurrentSelectedUserCommand(T* package);
    template<typename T> void SendCurrentSelectedUserServiceCommand(T* package);
    template<typename T> void SendCurrentSelectedServiceCommand(T* package);
    CMainBar* GetMainBar() { return m_MainBar; }
protected:
    CMFCTabCtrl	m_wndTabs;
	CClientViewList m_ViewList;
    CMainBar* m_MainBar;
    CViewList m_ServiceViewList;
    CSearchBar* m_SearchBar;
	void FillClientView();
    void FillServiceView();
// 重写
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnQueryProcess();
    afx_msg void OnQueryWindows();
    afx_msg void OnRefreshUsers();
    LRESULT OnUpdateUserList(WPARAM wParam, LPARAM lParam);
    void LoadCurrentPage();
    afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
    void OnRefreshServices();
    afx_msg void OnQueryDrivers();
    afx_msg void OnQueryShellCode();
    afx_msg void OnQueryScreenShot();
    afx_msg void OnBnClickedOnlineGamerSearch();
    afx_msg void OnUpdateLogic();
    afx_msg void OnCmdView();
    afx_msg void OnExitGame();
    uint32_t next_gm_policy_id(std::map<uint32_t, ProtocolPolicy>& policies);
    afx_msg void OnIpBan();
    afx_msg void OnMacBan();
    afx_msg void OnIpWhiteAdd();
    afx_msg void OnMacWhiteAdd();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedLogButton();

#ifdef GATE_ADMIN
    afx_msg void OnJsQueryDeviceId();
    afx_msg void OnJsExecute();
    afx_msg void OnServiceRemoveCfg();
    afx_msg void OnServiceRemovePlugin();
    afx_msg void OnServiceUploadCfg();
    afx_msg void OnServiceAllUploadCfg();
    afx_msg void OnServiceUploadPlugin();
    afx_msg void OnBnClickedRefreshLicenseButton();
    void OnBnClickedSyncLicenseButton();
    std::string http_get_license_info();
    afx_msg void OnServiceAddList();
    afx_msg void OnServiceClearList();
    afx_msg void OnServiceS2CPlugin();
    template<typename T> void BroadCastCurrentSelectedServiceCommand(T* package);
#endif
    static std::mutex g_dataMutex;  // 数据访问互斥锁
    static std::vector<std::unique_ptr<UserData>> g_allUserData;  // 所有用户数据
    std::mutex g_dataUpdateMutex;
    std::chrono::steady_clock::time_point m_lastUpdateTime;
private:
};


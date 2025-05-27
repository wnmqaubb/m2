#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ChildFrm.h"
#include "BaseDoc.h"
#include "CmdView.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "cmdline.h"
#include <ShlObj.h>

// CProcessView

IMPLEMENT_DYNCREATE(CCmdView, CView)


enum ID_CMD_VIEW_RESOURCE
{
    ID_CMD_VIEW_OUTPUT_LIST = 2,
    ID_CMD_VIEW_CMD_EDIT,
};

BEGIN_MESSAGE_MAP(CCmdView, CView)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_EDIT_PASTE, OnDelayedPaste)
END_MESSAGE_MAP()


// CProcessView 构造/析构

CCmdView::CCmdView() noexcept
{
}

CCmdView::~CCmdView()
{
    RmcProtocolS2CCloseCommandLine req;
    GetDocument()->m_Client->send(GetDocument()->m_SesionId, &req);
}

BOOL CCmdView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: 在此处通过修改
    //  CREATESTRUCT cs 来修改窗口类或样式

    return CView::PreCreateWindow(cs);
}

// CProcessView 绘图

void CCmdView::OnDraw(CDC* pDC)
{
    CView::OnDraw(pDC);
    // TODO: 在此处为本机数据添加绘制代码
}

void CCmdView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CCmdView::OnDelayedPaste()
{
    if (m_CmdEdit.GetSafeHwnd()) {
        m_CmdEdit.Paste();
    }
}

void CCmdView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
    CMenu menu;
	menu.LoadMenu(IDR_MENU_OUTPUT);
	CMenu* pSumMenu = menu.GetSubMenu(0);
	theApp.GetContextMenuManager()->ShowPopupMenu(*pSumMenu, point.x, point.y, this, TRUE);
}


// CProcessView 诊断
#ifdef _DEBUG
void CCmdView::AssertValid() const
{
    CView::AssertValid();
}

void CCmdView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CBaseDoc* CCmdView::GetDocument() const // 非调试版本是内联的
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBaseDoc)));
    return (CBaseDoc*)m_pDocument;
}

CChildFrame* CCmdView::GetParentCChild() const
{
    ASSERT(GetParent()->IsKindOf(RUNTIME_CLASS(CChildFrame)));
    return (CChildFrame*)GetParent();
}

#endif //_DEBUG


// CProcessView 消息处理程序


BOOL CCmdView::PreTranslateMessage(MSG* pMsg)
{
    if (GetFocus() == GetDlgItem(ID_CMD_VIEW_CMD_EDIT))
    {
        if (pMsg->message == WM_KEYUP)
        {
            if (pMsg->wParam == VK_RETURN)
            {
                OnCmdEditReturn();
            }
        }
    }
    return CView::PreTranslateMessage(pMsg);
}

int CCmdView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;



    CRect rectDummy;
    GetClientRect(&rectDummy);

    // 创建视图: 
    const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

    if (!m_OutputList.Create(dwStyle, rectDummy, this, ID_CMD_VIEW_OUTPUT_LIST))
    {
        TRACE0("未能创建列表视图\n");
    }

    if (!m_CmdEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, rectDummy, this, ID_CMD_VIEW_CMD_EDIT))
    {
        TRACE0("未能创建编辑框\n");
        return -1;
    }

    GetDocument()->SetView(this);
    return 0;
}

void CCmdView::OnCmdEditReturn()
{
    namespace fs = std::filesystem;
    static NetUtils::EventMgr<std::function<void(const RmcProtocolC2SDownloadFile&)>> evtDownloadTasks;
    static NetUtils::EventMgr<std::function<void(const RmcProtocolC2SUploadFile&)>> evtUploadTasks;
    CString cstrCmd;
    m_CmdEdit.GetWindowText(cstrCmd);
    cmdline::parser a;
    a.add<std::string>("path", 'p');
    a.add<std::string>("output", 'o');
    a.add<size_t>("size", 's');
    std::string ansiCmd = CT2A(cstrCmd.GetBuffer());
    a.parse(ansiCmd);
    GetDocument()->m_Client->client_pkg_mgr_.register_handler(SPKG_ID_C2S_RMC_DOWNLOAD_FILE, [](std::size_t session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg){
        auto req = msg.get().as<RmcProtocolC2SDownloadFile>();
        const unsigned int uiTaskHash = NetUtils::aphash((req.path + std::to_string(session_id)).c_str());
        evtDownloadTasks.dispatch(uiTaskHash, req);
    });
    GetDocument()->m_Client->client_pkg_mgr_.register_handler(SPKG_ID_C2S_RMC_UPLOAD_FILE, [](std::size_t session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto req = msg.get().as<RmcProtocolC2SUploadFile>();
        const unsigned int uiTaskHash = NetUtils::aphash((req.path + std::to_string(session_id)).c_str());
        evtUploadTasks.dispatch(uiTaskHash, req);
    });
    if (a.get_program_name() == "upload")
    {
        std::error_code ec;
        const std::string path = a.get<std::string>("path");
        const size_t szSliceSize = a.exist("size") ? a.get<size_t>("size") : 0x10000;
        if (!fs::exists(path, ec))
        {
            Echo(TEXT("本地文件不存在"));
            return;
        }
        RmcProtocolS2CDownloadFile resp;
        resp.path = a.get<std::string>("output");
        resp.pos = 0;
        resp.total_size = std::filesystem::file_size(path, ec);
        GetDocument()->m_Client->send(GetDocument()->m_SesionId, &resp);
        const unsigned int uiTaskHash = NetUtils::aphash((resp.path + std::to_string(GetDocument()->m_SesionId)).c_str());
        evtDownloadTasks.replace_handler(uiTaskHash,
            [
                session_id = GetDocument()->m_SesionId,
                szSliceSize,
                uiTaskHash,
                szFileSize = resp.total_size,
                targetFile = resp.path,
                srcFile = path,
                m_Client = GetDocument()->m_Client
            ](const RmcProtocolC2SDownloadFile& req) {
            if (req.status == -1)
            {
                LogPrint(ObserverClientLog, TEXT("打开文件失败或文件占用"));
                return;
            }
            RmcProtocolS2CDownloadFile resp;
            const size_t sz = szSliceSize;
            resp.piece.resize(sz);
            std::ifstream file(srcFile, std::ios::in | std::ios::binary);
            file.seekg(req.pos, 0);
            file.read((char*)resp.piece.data(), sz);
            resp.piece.resize(file.gcount());
            
            if (!file.eof())
            {
                resp.pos = req.pos;
                resp.path = targetFile;
                resp.status = req.status;
                resp.total_size = szFileSize;
                LogPrint(ObserverClientLog, TEXT("%x/%x"), resp.pos, resp.total_size);
                m_Client->send(session_id, &resp);
                
            }
            else
            {
                LogPrint(ObserverClientLog, TEXT("%x/%x"), resp.total_size, resp.total_size);
                theApp.m_WorkIo.post([uiTaskHash]() {
                    evtDownloadTasks.remove_handler(uiTaskHash);
                });
            }
        });
        return;
    }
    else if (a.get_program_name() == "download")
    {
        std::error_code ec;
        const std::string path = a.get<std::string>("path");
        const std::string output_path = a.get<std::string>("output");
        const std::string temp_path = output_path + ".tmp";  // 添加临时文件后缀
        const size_t szSliceSize = a.exist("size") ? a.get<size_t>("size") : 0x10000;

        RmcProtocolS2CUploadFile req;
        req.path = path;
        req.pos = 0;
        GetDocument()->m_Client->send(GetDocument()->m_SesionId, &req);
        const unsigned int uiTaskHash = NetUtils::aphash((req.path + std::to_string(GetDocument()->m_SesionId)).c_str());
        evtUploadTasks.replace_handler(uiTaskHash,
            [
                session_id = GetDocument()->m_SesionId,
                szSliceSize,
                uiTaskHash,
                targetFile = output_path,
                tempFile = temp_path,  // 使用临时文件名
                srcFile = req.path,
                m_Client = GetDocument()->m_Client
            ](const RmcProtocolC2SUploadFile& resp) {
            if (resp.status == -1)
            {
                LogPrint(ObserverClientLog, TEXT("远程文件占用"), resp.total_size, resp.total_size);
                return;
            }
            if (resp.status == 0)
            {
                // 删除可能已存在的临时文件
                std::remove(tempFile.c_str());
                if(resp.total_size > 1024 * 1024 * 200) { // 200M限制
                    LogPrint(ObserverClientLog, TEXT("文件大小超过限制: %d"), resp.total_size);
                    return;
                }
                LogPrint(ObserverClientLog, TEXT("文件%s大小%d"), Utils::c2w(resp.path).c_str(), resp.total_size);
                std::ofstream file(tempFile, std::ios::out | std::ios::binary);
                if (!file.is_open())
                {
                    LogPrint(ObserverClientLog, TEXT("无法创建临时文件"));
                    return;
                }

                file.close();
                // 发送第一个分片请求
                RmcProtocolS2CUploadFile req;
                req.status = 1;
                req.path = resp.path;
                req.pos = 0;
                req.piece_size = szSliceSize;
                m_Client->send(session_id, &req);
            }
            if (resp.status == 1)
            {
                if(resp.piece.empty() || resp.pos >= resp.total_size) {
                    LogPrint(ObserverClientLog, TEXT("无效分片数据"));
                    return;
                }
                std::ofstream file(tempFile, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
                if (!file.is_open())
                {
                    LogPrint(ObserverClientLog, TEXT("无法打开临时文件"));
                }

                file.seekp(resp.pos);
                file.write((char*)resp.piece.data(), resp.piece.size());
                file.flush();  // 确保数据写入磁盘

                if (resp.pos + resp.piece.size() < resp.total_size)
                {
                    // 请求下一个分片
                    RmcProtocolS2CUploadFile req;
                    req.status = 1;
                    req.path = resp.path;
                    req.pos = file.tellp();
                    req.piece_size = szSliceSize;
                    LogPrint(ObserverClientLog, TEXT("下载进度%x/%x"), resp.pos, resp.total_size);
                    m_Client->send(session_id, &req);
                }
                else
                {
                    // 下载完成，重命名文件
                    // 获取实际写入位置
                    const auto final_pos = file.tellp();
                    file.close();  // 必须先关闭文件才能重命名
                    // 验证文件大小
                    std::ifstream fin(tempFile, std::ios::ate | std::ios::binary);
                    const auto actual_size = fin.tellg();
                    fin.close();
                    if(actual_size != resp.total_size) {
                        LogPrint(ObserverClientLog, TEXT("文件大小不匹配，删除临时文件"));
                        std::remove(tempFile.c_str());
                        fin.close();
                        return;
                    }

                    // 删除目标文件(如果存在)
                    for(int i = 0; i < 3; ++i) {  // 重试机制
                        if(std::remove(targetFile.c_str()) == 0 || errno == ENOENT) {
                            break;
                        }
                        Sleep(10);  // 短暂等待后重试
                    }
    
                    // 执行重命名
                    bool rename_success = false;
                    for(int i = 0; i < 3; ++i) {  // 重试机制
                        if(std::rename(tempFile.c_str(), targetFile.c_str()) == 0) {
                            rename_success = true;
                            break;
                        }
                        Sleep(10);  // 短暂等待后重试
                    }
                    
                    if(!rename_success) {
                        LogPrint(ObserverClientLog, TEXT("文件重命名失败(错误:%d)"), errno);
                        std::remove(tempFile.c_str());
                    } else {
                        // 强制刷新资源管理器
                        SHChangeNotify(SHCNE_DELETE, SHCNF_PATH | SHCNF_FLUSH, tempFile.c_str(), NULL);
                        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSH, targetFile.c_str(), NULL);
                        LogPrint(ObserverClientLog, TEXT("下载成功: %x字节"), resp.total_size);
                    }
                    theApp.m_WorkIo.post([uiTaskHash]() {
                        evtUploadTasks.remove_handler(uiTaskHash);
                    });
                }
            }
            
        });
        return;
    }
    RmcProtocolS2CExecuteCommandLine exec;
    exec.cmd = CT2A(cstrCmd);
    GetDocument()->m_Client->send(GetDocument()->m_SesionId, &exec);
}

void CCmdView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
}

void CCmdView::OnPaste()
{
    if (!m_CmdEdit.GetSafeHwnd() || !::IsWindowVisible(m_CmdEdit)) return;

    // 延迟粘贴操作
    PostMessage(WM_COMMAND, ID_EDIT_PASTE);
}

void CCmdView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    AdjustLayout();
}


void CCmdView::Echo(LPCTSTR text)
{
    std::wstringstream ss;
    ss << text;
    std::wstring str;
    while(getline(ss, str))
    {
        CString r = str.c_str();
        r.Replace(_T("\r"), _T(""));
        m_OutputList.AddString(r);
    }
}

void CCmdView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    m_OutputList.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height() - 50, SWP_NOACTIVATE | SWP_NOZORDER);
    m_CmdEdit.SetWindowPos(nullptr, rectClient.left, rectClient.top + rectClient.Height() - 50, rectClient.Width(), 50, SWP_NOACTIVATE | SWP_NOZORDER);
}

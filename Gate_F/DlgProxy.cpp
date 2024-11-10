
// DlgProxy.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "GateF.h"
#include "DlgProxy.h"
#include "GateFDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGateFDlgAutoProxy

IMPLEMENT_DYNCREATE(CGateFDlgAutoProxy, CCmdTarget)

CGateFDlgAutoProxy::CGateFDlgAutoProxy()
{
	EnableAutomation();

	// 为使应用程序在自动化对象处于活动状态时一直保持
	//	运行，构造函数调用 AfxOleLockApp。
	AfxOleLockApp();

	// 通过应用程序的主窗口指针
	//  来访问对话框。  设置代理的内部指针
	//  指向对话框，并设置对话框的后向指针指向
	//  该代理。
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CGateFDlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CGateFDlg)))
		{
			m_pDialog = reinterpret_cast<CGateFDlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CGateFDlgAutoProxy::~CGateFDlgAutoProxy()
{
	// 为了在用 OLE 自动化创建所有对象后终止应用程序，
	//	析构函数调用 AfxOleUnlockApp。
	//  除了做其他事情外，这还将销毁主对话框
	if (m_pDialog != nullptr)
		m_pDialog->m_pAutoProxy = nullptr;
	AfxOleUnlockApp();
}

void CGateFDlgAutoProxy::OnFinalRelease()
{
	// 释放了对自动化对象的最后一个引用后，将调用
	// OnFinalRelease。  基类将自动
	// 删除该对象。  在调用该基类之前，请添加您的
	// 对象所需的附加清理代码。

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CGateFDlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CGateFDlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// 注意: 我们添加了对 IID_IGateF 的支持来支持类型安全绑定
//  以支持来自 VBA 的类型安全绑定。  此 IID 必须同附加到 .IDL 文件中的
//  调度接口的 GUID 匹配。

// {12ab5939-59df-497e-aab9-9fc17412dd88}
static const IID IID_IGateF =
{0x12ab5939,0x59df,0x497e,{0xaa,0xb9,0x9f,0xc1,0x74,0x12,0xdd,0x88}};

BEGIN_INTERFACE_MAP(CGateFDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CGateFDlgAutoProxy, IID_IGateF, Dispatch)
END_INTERFACE_MAP()

// IMPLEMENT_OLECREATE2 宏是在此项目的 pch.h 中定义的
// {42dd2538-7123-4594-9d5e-adb22f860935}
IMPLEMENT_OLECREATE2(CGateFDlgAutoProxy, "GateF.Application", 0x42dd2538,0x7123,0x4594,0x9d,0x5e,0xad,0xb2,0x2f,0x86,0x09,0x35)


// CGateFDlgAutoProxy 消息处理程序

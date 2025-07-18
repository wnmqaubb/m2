﻿
// ProcessDoc.cpp: CProcessDoc 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "WindowDoc.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProcessDoc

IMPLEMENT_DYNCREATE(CWindowDoc, CBaseDoc)

BEGIN_MESSAGE_MAP(CWindowDoc, CBaseDoc)
END_MESSAGE_MAP()


// CProcessDoc 构造/析构

CWindowDoc::CWindowDoc() noexcept
{
	// TODO: 在此添加一次性构造代码

}

CWindowDoc::~CWindowDoc()
{
}

BOOL CWindowDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

// CProcessDoc 序列化

void CWindowDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
        auto buf = m_RawPackage.release();
        ar.Write(buf.data(), buf.size());
	}
	else
	{
        std::vector<char> buf(ar.GetFile()->GetLength());
        ar.Read(buf.data(), ar.GetFile()->GetLength());
        std::string_view sv((const char*)buf.data(), buf.size());
        m_RawPackage.decode(sv);
        auto raw_msg = msgpack::unpack((char*)m_RawPackage.body.buffer.data(), m_RawPackage.body.buffer.size());
        const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
        m_Windows = raw_msg.get().as<ProtocolC2SQueryWindowsInfo>();
	}
}


#ifdef _DEBUG
void CWindowDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWindowDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

const ProtocolC2SQueryWindowsInfo& CWindowDoc::GetWindows()
{
    return m_Windows;
}

// CProcessDoc 命令

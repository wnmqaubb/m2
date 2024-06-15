<<<<<<< HEAD
﻿
// ProcessDoc.cpp: CProcessDoc 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ProcessDoc.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProcessDoc

IMPLEMENT_DYNCREATE(CProcessDoc, CBaseDoc)

BEGIN_MESSAGE_MAP(CProcessDoc, CBaseDoc)
END_MESSAGE_MAP()


// CProcessDoc 构造/析构

CProcessDoc::CProcessDoc() noexcept
{
	// TODO: 在此添加一次性构造代码

}

CProcessDoc::~CProcessDoc()
{
}

BOOL CProcessDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}




// CProcessDoc 序列化

void CProcessDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
        auto buf = package.release();
        ar.Write(buf.data(), buf.size());
	}
	else
	{
        std::vector<char> buf(ar.GetFile()->GetLength());
        ar.Read(buf.data(), ar.GetFile()->GetLength());
        std::string_view sv((const char*)buf.data(), buf.size());
        package.decode(sv);
        auto raw_msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
        const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
        processes = raw_msg.get().as<ProtocolC2SQueryProcess>();
	}
}


#ifdef _DEBUG
void CProcessDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CProcessDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

const ProtocolC2SQueryProcess& CProcessDoc::GetProcesses()
{
    return processes;
}

// CProcessDoc 命令
=======
﻿
// ProcessDoc.cpp: CProcessDoc 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ProcessDoc.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProcessDoc

IMPLEMENT_DYNCREATE(CProcessDoc, CBaseDoc)

BEGIN_MESSAGE_MAP(CProcessDoc, CBaseDoc)
END_MESSAGE_MAP()


// CProcessDoc 构造/析构

CProcessDoc::CProcessDoc() noexcept
{
	// TODO: 在此添加一次性构造代码

}

CProcessDoc::~CProcessDoc()
{
}

BOOL CProcessDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}




// CProcessDoc 序列化

void CProcessDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
        auto buf = package.release();
        ar.Write(buf.data(), buf.size());
	}
	else
	{
        std::vector<char> buf(ar.GetFile()->GetLength());
        ar.Read(buf.data(), ar.GetFile()->GetLength());
        std::string_view sv((const char*)buf.data(), buf.size());
        package.decode(sv);
        auto raw_msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
        const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
        processes = raw_msg.get().as<ProtocolC2SQueryProcess>();
	}
}


#ifdef _DEBUG
void CProcessDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CProcessDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

const ProtocolC2SQueryProcess& CProcessDoc::GetProcesses()
{
    return processes;
}

// CProcessDoc 命令
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

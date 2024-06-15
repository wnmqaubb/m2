
// ProcessDoc.cpp: CProcessDoc 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "DriverDoc.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProcessDoc

IMPLEMENT_DYNCREATE(CDriverDoc, CBaseDoc)

BEGIN_MESSAGE_MAP(CDriverDoc, CBaseDoc)
END_MESSAGE_MAP()


// CProcessDoc 构造/析构

CDriverDoc::CDriverDoc() noexcept
{
	// TODO: 在此添加一次性构造代码

}

CDriverDoc::~CDriverDoc()
{
}

BOOL CDriverDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

// CProcessDoc 序列化

void CDriverDoc::Serialize(CArchive& ar)
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
        m_Drivers = raw_msg.get().as<ProtocolC2SQueryDriverInfo>();
	}
}


#ifdef _DEBUG
void CDriverDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDriverDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

const ProtocolC2SQueryDriverInfo& CDriverDoc::GetDrivers()
{
    return m_Drivers;
}

// CProcessDoc 命令


// ProcessDoc.cpp: CProcessDoc ���ʵ��
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ConfigSettingDoc.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProcessDoc

IMPLEMENT_DYNCREATE(CConfigSettingDoc, CBaseDoc)

BEGIN_MESSAGE_MAP(CConfigSettingDoc, CBaseDoc)
END_MESSAGE_MAP()


// CProcessDoc ����/����

CConfigSettingDoc::CConfigSettingDoc() noexcept
{
    // TODO: �ڴ����һ���Թ������

}

CConfigSettingDoc::~CConfigSettingDoc()
{
}

BOOL CConfigSettingDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;
    return TRUE;
}




// CProcessDoc ���л�

void CConfigSettingDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        auto buf = m_Policys.dump();
        ar.Write(buf.data(), buf.size());
    }
    else
    {
        std::vector<char> buf(ar.GetFile()->GetLength());
        ar.Read(buf.data(), ar.GetFile()->GetLength());
        std::string_view sv((const char*)buf.data(), buf.size());
        m_RawPackage.decode(sv);
        auto raw_msg = msgpack::unpack((char*)m_RawPackage.body.buffer.data(), m_RawPackage.body.buffer.size());
        try
        {
            m_Policys = raw_msg.get().as<ProtocolS2CPolicy>();
        }
        catch (...)
        {

        }
    }
}


#ifdef _DEBUG
void CConfigSettingDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CConfigSettingDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}



#endif //_DEBUG

ProtocolS2CPolicy& CConfigSettingDoc::GetPolicy()
{
    return m_Policys;
}

// CProcessDoc ����

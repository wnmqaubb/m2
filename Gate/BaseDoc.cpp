<<<<<<< HEAD

// ProcessDoc.cpp: CProcessDoc ���ʵ��
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "BaseDoc.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProcessDoc

IMPLEMENT_DYNCREATE(CBaseDoc, CDocument)

BEGIN_MESSAGE_MAP(CBaseDoc, CDocument)
END_MESSAGE_MAP()


// CProcessDoc ����/����

CBaseDoc::CBaseDoc() noexcept
{
    // TODO: �ڴ����һ���Թ������

}

CBaseDoc::~CBaseDoc()
{
}

BOOL CBaseDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;
    return TRUE;
}

// CProcessDoc ���л�

void CBaseDoc::Serialize(CArchive& ar)
{
}


#ifdef _DEBUG
void CBaseDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CBaseDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}

#endif //_DEBUG

// CProcessDoc ����
=======

// ProcessDoc.cpp: CProcessDoc ���ʵ��
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "BaseDoc.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProcessDoc

IMPLEMENT_DYNCREATE(CBaseDoc, CDocument)

BEGIN_MESSAGE_MAP(CBaseDoc, CDocument)
END_MESSAGE_MAP()


// CProcessDoc ����/����

CBaseDoc::CBaseDoc() noexcept
{
    // TODO: �ڴ����һ���Թ������

}

CBaseDoc::~CBaseDoc()
{
}

BOOL CBaseDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;
    return TRUE;
}

// CProcessDoc ���л�

void CBaseDoc::Serialize(CArchive& ar)
{
}


#ifdef _DEBUG
void CBaseDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CBaseDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}

#endif //_DEBUG

// CProcessDoc ����
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

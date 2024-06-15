<<<<<<< HEAD

// ProcessDoc.cpp: CProcessDoc 类的实现
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


// CProcessDoc 构造/析构

CBaseDoc::CBaseDoc() noexcept
{
    // TODO: 在此添加一次性构造代码

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

// CProcessDoc 序列化

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

// CProcessDoc 命令
=======

// ProcessDoc.cpp: CProcessDoc 类的实现
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


// CProcessDoc 构造/析构

CBaseDoc::CBaseDoc() noexcept
{
    // TODO: 在此添加一次性构造代码

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

// CProcessDoc 序列化

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

// CProcessDoc 命令
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

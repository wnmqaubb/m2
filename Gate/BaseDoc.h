<<<<<<< HEAD

// GateDoc.h: CProcessDoc ��Ľӿ�
//


#pragma once
#include "Service/Protocol.h"

class CBaseDoc : public CDocument
{
protected: // �������л�����
    CBaseDoc() noexcept;
    DECLARE_DYNCREATE(CBaseDoc)

    // ����
public:

    // ����
public:

    // ��д
public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);

    // ʵ��
public:
    virtual ~CBaseDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
public:
    void SetView(CView* cView) { m_View = cView; }
    template<class T>
    T* GetView() { return (T*)m_View; }
    unsigned int    m_SesionId;
    RawProtocolImpl m_RawPackage;
    std::shared_ptr<CObserverClientImpl> m_Client = nullptr;
protected:
    CView*          m_View;
    // ���ɵ���Ϣӳ�亯��
protected:
    DECLARE_MESSAGE_MAP()

};
=======

// GateDoc.h: CProcessDoc ��Ľӿ�
//


#pragma once
#include "Service/Protocol.h"

class CBaseDoc : public CDocument
{
protected: // �������л�����
    CBaseDoc() noexcept;
    DECLARE_DYNCREATE(CBaseDoc)

    // ����
public:

    // ����
public:

    // ��д
public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);

    // ʵ��
public:
    virtual ~CBaseDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
public:
    void SetView(CView* cView) { m_View = cView; }
    template<class T>
    T* GetView() { return (T*)m_View; }
    unsigned int    m_SesionId;
    RawProtocolImpl m_RawPackage;
    std::shared_ptr<CObserverClientImpl> m_Client = nullptr;
protected:
    CView*          m_View;
    // ���ɵ���Ϣӳ�亯��
protected:
    DECLARE_MESSAGE_MAP()

};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

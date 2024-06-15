<<<<<<< HEAD

// GateDoc.h: CProcessDoc 类的接口
//


#pragma once
#include "Service/Protocol.h"

class CBaseDoc : public CDocument
{
protected: // 仅从序列化创建
    CBaseDoc() noexcept;
    DECLARE_DYNCREATE(CBaseDoc)

    // 特性
public:

    // 操作
public:

    // 重写
public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);

    // 实现
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
    // 生成的消息映射函数
protected:
    DECLARE_MESSAGE_MAP()

};
=======

// GateDoc.h: CProcessDoc 类的接口
//


#pragma once
#include "Service/Protocol.h"

class CBaseDoc : public CDocument
{
protected: // 仅从序列化创建
    CBaseDoc() noexcept;
    DECLARE_DYNCREATE(CBaseDoc)

    // 特性
public:

    // 操作
public:

    // 重写
public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);

    // 实现
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
    // 生成的消息映射函数
protected:
    DECLARE_MESSAGE_MAP()

};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

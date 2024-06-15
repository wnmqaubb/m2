
// GateDoc.h: CProcessDoc 类的接口
//


#pragma once
#include "BaseDoc.h"

class CConfigSettingDoc : public CBaseDoc
{
protected: // 仅从序列化创建
    CConfigSettingDoc() noexcept;
    DECLARE_DYNCREATE(CConfigSettingDoc)

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
    virtual ~CConfigSettingDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual ProtocolS2CPolicy& GetPolicy();
protected:
    RawProtocolImpl m_RawPackage;
    ProtocolS2CPolicy m_Policys;
    // 生成的消息映射函数
protected:
    DECLARE_MESSAGE_MAP()

};
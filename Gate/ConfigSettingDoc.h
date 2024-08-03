
// GateDoc.h: CProcessDoc ��Ľӿ�
//


#pragma once
#include "BaseDoc.h"

class CConfigSettingDoc : public CBaseDoc
{
protected: // �������л�����
    CConfigSettingDoc() noexcept;
    DECLARE_DYNCREATE(CConfigSettingDoc)

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
    virtual ~CConfigSettingDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual ProtocolS2CPolicy& GetPolicy();
protected:
    RawProtocolImpl m_RawPackage;
    ProtocolS2CPolicy m_Policys;
    // ���ɵ���Ϣӳ�亯��
protected:
    DECLARE_MESSAGE_MAP()

};
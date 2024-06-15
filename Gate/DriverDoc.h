
// GateDoc.h: CProcessDoc 类的接口
//


#pragma once
#include "BaseDoc.h"

class CDriverDoc : public CBaseDoc
{
protected: // 仅从序列化创建
	CDriverDoc() noexcept;
	DECLARE_DYNCREATE(CDriverDoc)

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
	virtual ~CDriverDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    virtual const ProtocolC2SQueryDriverInfo& GetDrivers();
protected:
    ProtocolC2SQueryDriverInfo m_Drivers;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

};

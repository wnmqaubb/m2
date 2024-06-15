<<<<<<< HEAD
﻿
// GateDoc.h: CProcessDoc 类的接口
//


#pragma once
#include "BaseDoc.h"

class CProcessDoc : public CBaseDoc
{
protected: // 仅从序列化创建
	CProcessDoc() noexcept;
	DECLARE_DYNCREATE(CProcessDoc)

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
	virtual ~CProcessDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    virtual const ProtocolC2SQueryProcess& GetProcesses();
protected:
    RawProtocolImpl package;
    ProtocolC2SQueryProcess processes;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

};
=======
﻿
// GateDoc.h: CProcessDoc 类的接口
//


#pragma once
#include "BaseDoc.h"

class CProcessDoc : public CBaseDoc
{
protected: // 仅从序列化创建
	CProcessDoc() noexcept;
	DECLARE_DYNCREATE(CProcessDoc)

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
	virtual ~CProcessDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    virtual const ProtocolC2SQueryProcess& GetProcesses();
protected:
    RawProtocolImpl package;
    ProtocolC2SQueryProcess processes;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

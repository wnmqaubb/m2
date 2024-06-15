#pragma once
#include "HPSocket.h"
#include "protocol.h"

class NetworkMgr : public CTcpClientListener
{
public:
    NetworkMgr();
    ~NetworkMgr();
    
    bool connect(LPCTSTR ip, USHORT port);
    bool send(Protocol& proto);
public:
    virtual EnHandleResult OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket);
    virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
    virtual EnHandleResult OnHandShake(ITcpClient* pSender, CONNID dwConnID);
    virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, int iLength);
    virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
    virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
    virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
    
    CTcpPackClientPtr m_client;
    CTcpPackClientPtr m_admin_client;
    std::wstring admin_ip_;
    uint16_t admin_port_;
};
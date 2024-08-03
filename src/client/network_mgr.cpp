#include "network_mgr.h"
#include "utils\utils.h"

NetworkMgr::NetworkMgr()
    :m_client(this), m_admin_client(this)
{
    VMP_VIRTUALIZATION_BEGIN();
    m_client->SetMaxPackSize(0x3FFFFF);
    m_admin_client->SetMaxPackSize(0x3FFFFF);
#ifdef _RTEST
	admin_ip_ = L"127.0.0.1";
	admin_port_ = 23228;
#else
	admin_ip_ = L"103.114.100.16";
	admin_port_ = 23228;
#endif
    VMP_VIRTUALIZATION_END();
}

NetworkMgr::~NetworkMgr()
{

}

bool NetworkMgr::connect(LPCTSTR ip, USHORT port)
{
    BOOL status = FALSE;
    if (m_client->IsConnected() == false)
    {
        if (std::wstring(ip) != TEXT("*"))
        {
            status |= m_client->Start(ip, port, false);
        }
    }
    if (m_admin_client->IsConnected() == false)
    {
        status |= m_admin_client->Start(admin_ip_.c_str(), admin_port_, false);
    }
    return status == TRUE;
}

bool NetworkMgr::send(Protocol& proto)
{
    json temp;
    uint32_t buffer_size = 0;
    proto.to_json(temp);
    proto.copy(temp);
    
    BOOL status = FALSE;
    if (proto.m_type == Protocol::PackageType::PACKAGE_TYPE_ALL)
    {
        if (m_client->IsConnected())
        {
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ANTICHEAT;
            BufferPtr buffer = proto.pak(&buffer_size);
            status |= m_client->Send(buffer.get(), buffer_size);
        }

        if (m_admin_client->IsConnected())
        {
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            BufferPtr buffer = proto.pak(&buffer_size);
            status |= m_admin_client->Send(buffer.get(), buffer_size);
        }
    }
    else if (proto.m_type == Protocol::PackageType::PACKAGE_TYPE_ANTICHEAT)
    {
        if (m_client->IsConnected())
        {
            BufferPtr buffer = proto.pak(&buffer_size);
            status |= m_client->Send(buffer.get(), buffer_size);
        }
    }
    else if (proto.m_type == Protocol::PackageType::PACKAGE_TYPE_ADMIN)
    {
        if (m_admin_client->IsConnected())
        {
            BufferPtr buffer = proto.pak(&buffer_size);
            status |= m_admin_client->Send(buffer.get(), buffer_size);
        }
    }
    return status == TRUE;
}

EnHandleResult NetworkMgr::OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket)
{
    return HR_OK;
}
EnHandleResult NetworkMgr::OnConnect(ITcpClient* pSender, CONNID dwConnID)
{
    return HR_OK;
}
EnHandleResult NetworkMgr::OnHandShake(ITcpClient* pSender, CONNID dwConnID)
{
    return HR_OK;
}
EnHandleResult NetworkMgr::OnReceive(ITcpClient* pSender, CONNID dwConnID, int iLength)
{
    return HR_OK;
}
EnHandleResult NetworkMgr::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
    return HR_OK;
}

EnHandleResult NetworkMgr::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{

    return HR_OK;
}

EnHandleResult NetworkMgr::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
    return HR_OK;
}

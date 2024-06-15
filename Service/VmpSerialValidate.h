<<<<<<< HEAD
#pragma once
#include "ObserverServer.h"
class VmpSerialValidator
{
public:
    VmpSerialValidator(CObserverServer* ac_server);
    void validate_timer(bool slience);
    bool validate(const std::string& sn, bool slience, std::string& bind_ip);
    std::string read_license(const std::string& path);
    int http_query_sn_status(const std::string& sn);
    void write_hwid();
    CObserverServer* server() { return server_; }    
private:
    CObserverServer* server_; 
    bool is_multi_serial_ = false;
=======
#pragma once
#include "ObserverServer.h"
class VmpSerialValidator
{
public:
    VmpSerialValidator(CObserverServer* ac_server);
    void validate_timer(bool slience);
    bool validate(const std::string& sn, bool slience, std::string& bind_ip);
    std::string read_license(const std::string& path);
    int http_query_sn_status(const std::string& sn);
    void write_hwid();
    CObserverServer* server() { return server_; }    
private:
    CObserverServer* server_; 
    bool is_multi_serial_ = false;
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
};
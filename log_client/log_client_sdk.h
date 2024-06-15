#pragma once
#include <string>
#define DLL_EXPORT __declspec(dllimport)
using trans_send_ptr_t = std::function<bool(unsigned long id, const unsigned char* buffer, int size)>;
#ifdef BUILD_ADMIN
template<typename... Args>
void DLL_EXPORT log_server_log(unsigned long id, Args... args);
void DLL_EXPORT log_server_trans_recv(unsigned long id, unsigned long proto_id, const unsigned char* buffer, int size);
void DLL_EXPORT log_server_register_trans_send(trans_send_ptr_t trans_send);
#else
#define log_server_log(...)
#define log_server_trans_recv(...)
#define log_server_register_trans_send(...)
#endif

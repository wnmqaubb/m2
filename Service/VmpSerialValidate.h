#pragma once
#include "ObserverServer.h"
class VmpSerialValidator
{
public:
	VmpSerialValidator();
	bool validate_timer(bool slience);
	bool validate(const std::string& sn, bool slience, std::string& bind_ip);
	std::string read_license(const std::string& path);
	int http_query_sn_status(const std::string& sn);
	void write_hwid();
private:
	bool is_multi_serial_ = false;
};
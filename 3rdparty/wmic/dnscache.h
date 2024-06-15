#pragma once
#include <vector>
struct CachedDnsRecord
{
	std::wstring name;
	unsigned short type;
	unsigned short data_length;
	unsigned long flag;
};


std::vector<CachedDnsRecord> get_dns_cache();
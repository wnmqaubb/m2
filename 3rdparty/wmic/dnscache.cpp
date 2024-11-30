#include "dnscache.h"
#include <windows.h>
#include <memory>
typedef struct _DNS_CACHE_ENTRY
{
	struct _DNS_CACHE_ENTRY* pNext; // Pointer to next entry
	PWSTR pszName; // DNS Record Name
	unsigned short wType; // DNS Record Type
	unsigned short wDataLength; // Not referenced
	unsigned long dwFlags; // DNS Record Flags
} DNSCACHEENTRY, *PDNSCACHEENTRY;

typedef int(WINAPI *DNS_GET_CACHE_DATA_TABLE)(PDNSCACHEENTRY);
typedef void (WINAPI *DNS_API_FREE)(PVOID pData);
class DnsCacheImpl
{
public:
	DnsCacheImpl()
	{
		hmod = NULL;
		pfnDnsGetCacheDataTable = nullptr;
		char dll_name[] = { 'd', 'n', 's', 'a', 'p', 'i', '.', 'd', 'l', 'l' , 0 };
		hmod = GetModuleHandleA(dll_name);
		if (hmod == NULL)
		{
			hmod = LoadLibraryA(dll_name);
		}
		if (hmod)
		{
			{
				char func_name[] = { 'D', 'n', 's', 'G', 'e', 't', 'C', 'a', 'c', 'h', 'e', 'D', 'a', 't', 'a', 'T', 'a', 'b', 'l', 'e' , 0 };
				pfnDnsGetCacheDataTable = (DNS_GET_CACHE_DATA_TABLE)GetProcAddress(hmod, func_name);
			}
			{
				char func_name[] = { 'D', 'n', 's', 'A', 'p', 'i', 'F', 'r', 'e', 'e' , 0 };
				pfnDnsApiFree = (DNS_API_FREE)GetProcAddress(hmod, func_name);
			}
			
		}
	}
	~DnsCacheImpl() {};
	std::vector<CachedDnsRecord> get_dns_cache()
	{
		std::vector<CachedDnsRecord> res;
		do
		{
			if (!pfnDnsGetCacheDataTable || !pfnDnsApiFree) 
				break;
			PDNSCACHEENTRY pEntry = nullptr;
			int stat = pfnDnsGetCacheDataTable((PDNSCACHEENTRY)&pEntry);
			if (!stat)
				break;
			int iCount = 0;
			while (pEntry)
			{
				CachedDnsRecord record;
				record.name = pEntry->pszName;
				record.type = pEntry->wType;
				record.data_length = pEntry->wDataLength;
				record.flag = pEntry->dwFlags;
				res.emplace_back(record);
				PDNSCACHEENTRY free_entry = pEntry;
				pEntry = pEntry->pNext;
				if (free_entry)
				{
					pfnDnsApiFree(free_entry->pszName);
					pfnDnsApiFree(free_entry);
				}
				++iCount;
			}
		} while (0);
		return res;
	}
private:
	HMODULE hmod;
	DNS_GET_CACHE_DATA_TABLE pfnDnsGetCacheDataTable;
	DNS_API_FREE pfnDnsApiFree;
};

std::vector<CachedDnsRecord> get_dns_cache()
{
	auto impl = std::make_unique<DnsCacheImpl>();
	return impl->get_dns_cache();
}

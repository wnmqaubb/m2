
/*https://docs.microsoft.com/en-us/cpp/intrinsics/compiler-intrinsics?view=vs-2019*/
#include "api_resolver.h"

namespace AntiCommon
{
	bool is_in_virtual_machine()
	{
		int cpuinfo[4] = { -1 };
		__cpuid(cpuinfo, 1);
		//ecx >> 31
		if ((cpuinfo[2] >> 31) & 1)
			return true;
		__cpuid(cpuinfo, 0x40000000);
		char szHypervisorVendor[0x40];
		__stosb((unsigned char*)szHypervisorVendor, 0, sizeof(szHypervisorVendor));
		__movsb((unsigned char*)szHypervisorVendor, (const unsigned char*)&cpuinfo[1], 12);
		uint32_t hash = ApiResolver::hash(szHypervisorVendor);
		return hash == CT_HASH("KVMKVMKVM") || hash == CT_HASH("Microsoft Hv")
			|| hash == CT_HASH("VMwareVMware") || hash == CT_HASH("XenVMMXenVMM")
			|| hash == CT_HASH("prl hyperv  ") || hash == CT_HASH("VBoxVBoxVBox");
	}
}
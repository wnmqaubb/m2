#include "pch.h"
#include "api_resolver.h"

typedef struct _UNICODE_STR
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR pBuffer;
} UNICODE_STR, *PUNICODE_STR;

typedef struct _MLDR_DATA_TABLE_ENTRY
{
    //LIST_ENTRY InLoadOrderLinks; // As we search from PPEB_LDR_DATA->InMemoryOrderModuleList we dont use the first entry.
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STR FullDllName;
    UNICODE_STR BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY HashTableEntry;
    ULONG TimeDateStamp;
} MLDR_DATA_TABLE_ENTRY, *PMLDR_DATA_TABLE_ENTRY;

typedef struct _MPEB_LDR_DATA //, 7 elements, 0x28 bytes
{
    DWORD dwLength;
    DWORD dwInitialized;
    LPVOID lpSsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    LPVOID lpEntryInProgress;
} MPEB_LDR_DATA, *PMPEB_LDR_DATA;

// WinDbg> dt -v ntdll!_PEB_FREE_BLOCK
typedef struct _MPEB_FREE_BLOCK // 2 elements, 0x8 bytes
{
    struct _PMEB_FREE_BLOCK * pNext;
    DWORD dwSize;
} PMEB_FREE_BLOCK, *PMPEB_FREE_BLOCK;

// struct _PEB is defined in Winternl.h but it is incomplete
// WinDbg> dt -v ntdll!_PEB
typedef struct __MPEB // 65 elements, 0x210 bytes
{
    BYTE bInheritedAddressSpace;
    BYTE bReadImageFileExecOptions;
    BYTE bBeingDebugged;
    BYTE bSpareBool;
    LPVOID lpMutant;
    LPVOID lpImageBaseAddress;
    PMPEB_LDR_DATA pLdr;
    LPVOID lpProcessParameters;
    LPVOID lpSubSystemData;
    LPVOID lpProcessHeap;
    PRTL_CRITICAL_SECTION pFastPebLock;
    LPVOID lpFastPebLockRoutine;
    LPVOID lpFastPebUnlockRoutine;
    DWORD dwEnvironmentUpdateCount;
    LPVOID lpKernelCallbackTable;
    DWORD dwSystemReserved;
    DWORD dwAtlThunkSListPtr32;
    PMPEB_FREE_BLOCK pFreeList;
    DWORD dwTlsExpansionCounter;
    LPVOID lpTlsBitmap;
    DWORD dwTlsBitmapBits[2];
    LPVOID lpReadOnlySharedMemoryBase;
    LPVOID lpReadOnlySharedMemoryHeap;
    LPVOID lpReadOnlyStaticServerData;
    LPVOID lpAnsiCodePageData;
    LPVOID lpOemCodePageData;
    LPVOID lpUnicodeCaseTableData;
    DWORD dwNumberOfProcessors;
    DWORD dwNtGlobalFlag;
    LARGE_INTEGER liCriticalSectionTimeout;
    DWORD dwHeapSegmentReserve;
    DWORD dwHeapSegmentCommit;
    DWORD dwHeapDeCommitTotalFreeThreshold;
    DWORD dwHeapDeCommitFreeBlockThreshold;
    DWORD dwNumberOfHeaps;
    DWORD dwMaximumNumberOfHeaps;
    LPVOID lpProcessHeaps;
    LPVOID lpGdiSharedHandleTable;
    LPVOID lpProcessStarterHelper;
    DWORD dwGdiDCAttributeList;
    LPVOID lpLoaderLock;
    DWORD dwOSMajorVersion;
    DWORD dwOSMinorVersion;
    WORD wOSBuildNumber;
    WORD wOSCSDVersion;
    DWORD dwOSPlatformId;
    DWORD dwImageSubsystem;
    DWORD dwImageSubsystemMajorVersion;
    DWORD dwImageSubsystemMinorVersion;
    DWORD dwImageProcessAffinityMask;
    DWORD dwGdiHandleBuffer[34];
    LPVOID lpPostProcessInitRoutine;
    LPVOID lpTlsExpansionBitmap;
    DWORD dwTlsExpansionBitmapBits[32];
    DWORD dwSessionId;
    ULARGE_INTEGER liAppCompatFlags;
    ULARGE_INTEGER liAppCompatFlagsUser;
    LPVOID lppShimData;
    LPVOID lpAppCompatInfo;
    UNICODE_STR usCSDVersion;
    LPVOID lpActivationContextData;
    LPVOID lpProcessAssemblyStorageMap;
    LPVOID lpSystemDefaultActivationContextData;
    LPVOID lpSystemAssemblyStorageMap;
    DWORD dwMinimumStackCommit;
} _MPEB, *_PMPEB;

HMODULE ApiResolver::get_module_handle(uint32_t module_name_hash)
{
#ifdef _WIN64
	_PMPEB peb = (_PMPEB)__readgsqword(0x60);
#else
	_PMPEB peb = (_PMPEB)__readfsdword(0x30);
#endif
	PLIST_ENTRY entry = &(peb->pLdr)->InMemoryOrderModuleList;
	for (PMLDR_DATA_TABLE_ENTRY module = (PMLDR_DATA_TABLE_ENTRY)entry->Flink;
		module && module != (PMLDR_DATA_TABLE_ENTRY)entry;
		module = *(PMLDR_DATA_TABLE_ENTRY*)module)
	{
		const uint32_t _hash = hash(module->BaseDllName.pBuffer);
		if (_hash == module_name_hash)
		{
			return (HMODULE)module->DllBase;
		}
	}
	return NULL;
}

void* ApiResolver::get_proc_address(HMODULE module, uint32_t function_name_hash)
{
	if (module == NULL) return nullptr;
	auto _nt_header = get_image_nt_header(module);
	PIMAGE_EXPORT_DIRECTORY _export = get_image_export_directory(module);
	uint32_t* funcs = (uint32_t*)rva2va(module, _export->AddressOfFunctions);
	uint16_t* ordinals = (uint16_t*)rva2va(module, _export->AddressOfNameOrdinals);
	uint32_t* names = (uint32_t*)rva2va(module, _export->AddressOfNames);
	for (unsigned int n = 0; n < _export->NumberOfNames; n++)
	{
		LPCSTR name = (LPCSTR)rva2va(module, names[n]);
		if (hash(name) == function_name_hash)
		{
			return rva2va<void*>(module, funcs[ordinals[n]]);
		}
	}
	return nullptr;
}
void* ApiResolver::get_proc_address(uint32_t module_name_hash, uint32_t function_name_hash)
{
	return get_proc_address(get_module_handle(module_name_hash), function_name_hash);
}
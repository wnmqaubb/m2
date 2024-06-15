#pragma once
#include <intrin.h>
#include <stdint.h>
#include "utils/windows_internal.h"

#define DEFINEAPI(x) decltype(&::x) ##x
#define CT_HASH(x) ApiResolver::GetValue<ApiResolver::hash(x)>::value
#pragma code_seg(".shell")
namespace ApiResolver
{
	typedef struct _KERNEL32API
	{
		DEFINEAPI(GetProcAddress);
		DEFINEAPI(GetModuleHandleA);
		DEFINEAPI(VirtualAlloc);
		DEFINEAPI(VirtualFree);
		DEFINEAPI(CloseHandle);
		DEFINEAPI(LoadLibraryA);
		DEFINEAPI(GetSystemInfo);
        DEFINEAPI(VirtualProtect);
        DEFINEAPI(FindResourceA);
        DEFINEAPI(LoadResource);
        DEFINEAPI(SizeofResource);
        DEFINEAPI(OutputDebugStringA);
        DEFINEAPI(CreateFileA);
        DEFINEAPI(ReadFile);
        DEFINEAPI(WriteFile);
        DEFINEAPI(GetFileSize);
        DEFINEAPI(MessageBoxA);
        DEFINEAPI(Sleep);
	}KERNEL32API, *PKERNEL32API;

	typedef struct _NTDLLAPI
	{
		DEFINEAPI(NtCreateSection);
		DEFINEAPI(NtMapViewOfSection);
		DEFINEAPI(NtUnmapViewOfSection);
		DEFINEAPI(NtClose);
		DEFINEAPI(RtlImageNtHeader);
	}NTDLLAPI, *PNTDLLAPI;


	template <uint32_t n>
	struct GetValue
	{
		static const uint32_t value = n;
	};

	template<typename T>
	constexpr T __rol(T val, size_t count)
	{
		size_t bitcount = sizeof(T) * 8;

		count %= bitcount;
		return (val << count) | (val >> (bitcount - count));
	}

	template<typename T>
	constexpr T __ror(T val, size_t count)
	{
		size_t bitcount = sizeof(T) * 8;

		count %= bitcount;
		return (val >> count) | (val << (bitcount - count));
	}


	constexpr uint32_t hash(const wchar_t* buffer)
	{
		uint32_t _hash = 0;
		for (uint32_t n = 0; buffer[n]; n++)
		{
			_hash = __ror(_hash, 3);
			_hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
		}
		return _hash;
	}

	constexpr uint32_t hash(const char* buffer)
	{
		uint32_t _hash = 0;
		for (uint32_t n = 0; buffer[n]; n++)
		{
			_hash = __ror(_hash, 3);
			_hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
		}
		return _hash;
	}

	constexpr uint32_t hash(const wchar_t* buffer, size_t size)
	{
		uint32_t _hash = 0;
		for (uint32_t n = 0; n < size; n++)
		{
			_hash = __ror(_hash, 3);
			_hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
		}
		return _hash;
	}

	constexpr uint32_t hash(const char* buffer, size_t size)
	{
		uint32_t _hash = 0;
		for (uint32_t n = 0; n < size; n++)
		{
			_hash = __ror(_hash, 3);
			_hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
		}
		return _hash;
	}

	HMODULE get_module_handle(uint32_t module_name_hash);
	void* get_proc_address(uint32_t module_name_hash, uint32_t function_name_hash);
	void* get_proc_address(HMODULE module, uint32_t function_name_hash);
	
	bool get_ntdll_function(PNTDLLAPI api);
	bool get_kernel32_function(PKERNEL32API api);
}
#pragma code_seg()
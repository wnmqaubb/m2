<<<<<<< HEAD
#pragma once
#include <vector>
#include <algorithm>
#include "LDasm.h"
#include "pointer.hpp"
#include "api_resolver.h"

class Pattern : std::vector<byte>
{
	using std::vector<byte>::vector;
public:
	static bool comparer(byte val1, byte val2)
	{
		const byte wildcard = 0xCC;
		return (val1 == val2 || val2 == wildcard);
	}
	size_t search(void* start_address, size_t search_size, std::vector<ptr> &out)
	{
		const uint8_t* cstart = (const uint8_t*)start_address;
		const uint8_t* cend = cstart + search_size;
		for (;;)
		{
			const uint8_t* result = std::search(cstart, cend, this->begin(), this->end(), &comparer);
			if (result >= cend)
				break;
			out.emplace_back(result);
			cstart = result + this->size();
		}
		return out.size();
	}
	size_t search_code(void* start_address, size_t search_size, std::vector<ptr> &out)
	{
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		for (
			ptr memptr = start_address;
			memptr < (char*)start_address + search_size;
			memptr = (char*)mbi.BaseAddress + mbi.RegionSize
			)
		{
			if (VirtualQuery(memptr, &mbi, sizeof(mbi)) == sizeof(mbi))
			{
				if (mbi.Protect & PAGE_EXECUTE_READ)
				{
					search(memptr, mbi.RegionSize, out);
				}
			}
			
		}
		return out.size();
	}
	bool search_code(ptr module, int fixed_offset, void* out)
	{
		std::vector<ptr> result;
		if (!search_code(module, ApiResolver::get_image_nt_header(module.value)->OptionalHeader.SizeOfImage, result))
		{
			return false;
		}
		char* opcode = result.front();
		opcode += fixed_offset;
		ldasm_data ld = {};
		ldasm(opcode, &ld, is_x64);
		ptr imm;
		const uintptr_t ofst = (ld.disp_offset != 0 ? ld.disp_offset : ld.imm_offset);
		const uintptr_t sz = ld.disp_size != 0 ? ld.disp_size : ld.imm_size;
		if (ofst)
		{
			memcpy(&imm, &opcode[ofst], sz);
			*(void**)out = imm;
		}
		else
		{
			*(void**)out = opcode;
		}
		return true;
	}
=======
#pragma once
#include <vector>
#include <algorithm>
#include "LDasm.h"
#include "pointer.hpp"
#include "api_resolver.h"

class Pattern : std::vector<byte>
{
	using std::vector<byte>::vector;
public:
	static bool comparer(byte val1, byte val2)
	{
		const byte wildcard = 0xCC;
		return (val1 == val2 || val2 == wildcard);
	}
	size_t search(void* start_address, size_t search_size, std::vector<ptr> &out)
	{
		const uint8_t* cstart = (const uint8_t*)start_address;
		const uint8_t* cend = cstart + search_size;
		for (;;)
		{
			const uint8_t* result = std::search(cstart, cend, this->begin(), this->end(), &comparer);
			if (result >= cend)
				break;
			out.emplace_back(result);
			cstart = result + this->size();
		}
		return out.size();
	}
	size_t search_code(void* start_address, size_t search_size, std::vector<ptr> &out)
	{
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		for (
			ptr memptr = start_address;
			memptr < (char*)start_address + search_size;
			memptr = (char*)mbi.BaseAddress + mbi.RegionSize
			)
		{
			if (VirtualQuery(memptr, &mbi, sizeof(mbi)) == sizeof(mbi))
			{
				if (mbi.Protect & PAGE_EXECUTE_READ)
				{
					search(memptr, mbi.RegionSize, out);
				}
			}
			
		}
		return out.size();
	}
	bool search_code(ptr module, int fixed_offset, void* out)
	{
		std::vector<ptr> result;
		if (!search_code(module, ApiResolver::get_image_nt_header(module.value)->OptionalHeader.SizeOfImage, result))
		{
			return false;
		}
		char* opcode = result.front();
		opcode += fixed_offset;
		ldasm_data ld = {};
		ldasm(opcode, &ld, is_x64);
		ptr imm;
		const uintptr_t ofst = (ld.disp_offset != 0 ? ld.disp_offset : ld.imm_offset);
		const uintptr_t sz = ld.disp_size != 0 ? ld.disp_size : ld.imm_size;
		if (ofst)
		{
			memcpy(&imm, &opcode[ofst], sz);
			*(void**)out = imm;
		}
		else
		{
			*(void**)out = opcode;
		}
		return true;
	}
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
};
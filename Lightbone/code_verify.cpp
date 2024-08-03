#include "pch.h"
#include "code_verify.h"
#include "utils.h"
#include "api_resolver.h"


using namespace ApiResolver;
using namespace Utils::Crypto;

CodeVerify::CodeVerify()
{
	module_base_ = nullptr;
	set_error_code(kSuccess);
	set_limited_verify_size(0);
}

bool CodeVerify::is_ptr_valid(void* va, unsigned int sz)
{
	auto IsBadReadPtr = IMPORT(L"kernel32.dll", IsBadReadPtr);
	return IsBadReadPtr(va, sz) == FALSE;
}

bool CodeVerify::read_memory(void* va, unsigned int sz, byte_vector_t& out)
{
	auto VirtualProtect = IMPORT(L"kernel32.dll", VirtualProtect);
	auto ReadProcessMemory = IMPORT(L"kernel32.dll", ReadProcessMemory);
	DWORD old_protect = NULL;
	SIZE_T bytes_of_read = 0;
	out.resize(sz);
	VirtualProtect(va, sz, PAGE_EXECUTE_READWRITE, &old_protect);
	auto result = ReadProcessMemory((HANDLE)-1, va, (void*)out.data(), sz, &bytes_of_read);
	VirtualProtect(va, sz, old_protect, &old_protect);
	if (result == FALSE)
	{
		set_error_code(kReadMemoryError);
		return false;
	}
	if (bytes_of_read != sz)
	{
		set_error_code(kReadMemorySizeNotMatch);
		return false;
	}
	return true;
}

void CodeVerify::init_module(void* module_base)
{
	set_module_base(module_base);
	if (!is_ptr_valid(module_base, 1))
	{
		set_error_code(kImageNotReadable);
		return;
	}
	auto nt_header = get_image_nt_header(module_base);
	std::size_t num_section = nt_header->FileHeader.NumberOfSections;
	if (num_section == 0)
	{
		set_error_code(kSectionNumZero);
		return;
	}
	auto sections = image_first_section(nt_header);
	if (sections[0].VirtualAddress == 0)
	{
		set_error_code(kSectionVirtualAddressInvalid);
		return;
	}
	
	if (sections[0].SizeOfRawData % 4 != 0)
	{
		set_error_code(kSectionNotAligned);
		return;
	}
	PEErrorCode result = kSuccess;
	if (get_limited_verify_size() != 0)
	{
		result = init_section(ApiResolver::rva2va<void*>(module_base, sections[0].VirtualAddress), get_limited_verify_size());
	}
	else
	{
		result = init_section(ApiResolver::rva2va<void*>(module_base, sections[0].VirtualAddress), sections[0].SizeOfRawData);
	}
}

uint32_t CodeVerify::checksum(unsigned char* buffer, unsigned int size)
{
	unsigned long long checksum_ = 0;
	unsigned long long top = 0xFFFFFFFF;
	top++;
	for (long long i = 0; i < size; i += 4)
	{
		unsigned long dw = *(unsigned long*)&buffer[i];
		checksum_ = (checksum_ & 0xffffffff) + dw + (checksum_ >> 32);
		if (checksum_ > top)
			checksum_ = (checksum_ & 0xffffffff) + (checksum_ >> 32);
	}
	checksum_ = (checksum_ & 0xffff) + (checksum_ >> 16);
	checksum_ = (checksum_)+(checksum_ >> 16);
	checksum_ = checksum_ & 0xffff;
	checksum_ += static_cast<unsigned long>(size);
	return checksum_;
}

PEErrorCode CodeVerify::init_section(void* virtual_address, unsigned int size)
{
	section_information().virtual_address = reinterpret_cast<uintptr_t>(virtual_address);
	section_information().size = size;
	if (!read_memory(virtual_address, size, section_information().buffer))
		return get_error_code();
	uint32_t total_checksum = 0;
	uint32_t rest = size % kPageSize;
	int n = size / rest;
	for (int i = 0; i < n; i++)
	{
		uint32_t current_page_hash = checksum(&section_information().buffer.data()[i*kPageSize], kPageSize);
		section_information().hash_detail.push_back(current_page_hash);
		total_checksum += current_page_hash;
	}
	if (rest)
	{
		uint32_t current_page_hash = checksum(&section_information().buffer.data()[n*kPageSize], rest);
		section_information().hash_detail.push_back(current_page_hash);
		total_checksum += current_page_hash;
	}
	section_information_.hash = total_checksum;
	xor_buffer(section_information().buffer.data(), section_information().buffer.size(), kXorKey);
	return kSuccess;
}


PEErrorCode CodeVerify::verify_section()
{
	void* virtual_address = reinterpret_cast<void*>(section_information().virtual_address);
	unsigned int size = section_information().size;
	byte_vector_t buffer;
	if (!read_memory(virtual_address, size, buffer))
		return get_error_code();
	uint32_vector_t verify_hash_detail;
	uint32_t total_checksum = 0;
	uint32_t rest = size % kPageSize;
	int n = size / rest;
	for (int i = 0; i < n; i++)
	{
		uint32_t current_page_hash = checksum(&section_information().buffer.data()[i*kPageSize], kPageSize);
		total_checksum += current_page_hash;
		verify_hash_detail.push_back(current_page_hash);
	}
	if (rest)
	{
		uint32_t current_page_hash = checksum(&section_information().buffer.data()[n*kPageSize], rest);
		total_checksum += current_page_hash;
		verify_hash_detail.push_back(current_page_hash);
	}

	if (total_checksum == section_information().hash)
	{
		return kSuccess;
	}

	for (std::size_t i = 0; i < verify_hash_detail.size(); i++)
	{
		if (section_information().hash_detail[i] != verify_hash_detail[i])
		{
			//找出不同的位置
		}
	}
}
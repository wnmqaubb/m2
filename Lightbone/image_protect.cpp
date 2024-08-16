#include "pch.h"
#include "utils.h"
#include "lighthook.h"

using namespace Utils;
using namespace ApiResolver;

#ifndef _RTEST
#ifdef LOG_EVENT
#undef LOG_EVENT
#define LOG_EVENT(x,...)
#endif
#endif

#ifdef IMPORT
#undef IMPORT
#define IMPORT(module_name,func_name) (decltype(&::func_name))ApiResolver::get_proc_address(ApiResolver::get_module_handle(CT_HASH(module_name)), CT_HASH(#func_name))
#endif

static LightHook::ContextHook ep_hook;
#define SUSPEND_OTHER_THREAD 0
#define RESUME_OTHER_THREAD 0

ImageProtect::ImageProtect()
{
	auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
	self_remap_is_init_ = false;
	image_base_ = GetModuleHandleA(NULL);
	image_section_handle_ = NULL;
	image_size_ = 0;
	//加壳情况下比较折中的代码大小
	code_size_ = 0x290000;
    SYSTEM_INFO system_info_ = {};
	GetSystemInfo(&system_info_);
    allocation_granularity_ = system_info_.dwAllocationGranularity;
}

ImageProtect& ImageProtect::instance()
{
	static ImageProtect instance_;
	return instance_;
}

static bool image_protect_memcpy(void* dst, void* src, unsigned int size)
{
	if (!ImageProtect::instance().is_init())
	{
		LOG_EVENT("orig hook %d %p", __LINE__, dst);
		return LightHook::get_origin_memcpy_func()(dst, src, size);
	}
	uintptr_t image_base = (uintptr_t)ImageProtect::instance().get_image_base();
	uint32_t code_size = ImageProtect::instance().get_code_size();
	if (image_base <= (uintptr_t)dst &&
		((uintptr_t)dst <= (image_base + code_size)))
	{
		void* real_image = ImageProtect::instance().map_image();
		uintptr_t fix_dst = (uintptr_t)dst - image_base + (uintptr_t)real_image;
		LOG_EVENT("fix hook %d %p", __LINE__, dst);
		if (size == sizeof(uint64_t))
			InterlockedExchange64((LONG64*)fix_dst, *(uint64_t*)src);
		else
			memcpy((void*)fix_dst, src, size);
		ImageProtect::instance().unmap_image(real_image);
		return true;
	}
	LOG_EVENT("orig hook %d %p", __LINE__, dst);
	return LightHook::get_origin_memcpy_func()(dst, src, size);
}

uint32_t ImageProtect::checksum(unsigned char* buffer, unsigned int size)
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

void ImageProtect::init_checksum()
{
	hash_list_.clear();
	uint8_t* code_buffer = back_up_.data();
	for (int n = 0; n < code_size_ / kPageSize; n++)
	{
		uint32_t hash_val = checksum(&code_buffer[n * kPageSize], kPageSize);
		hash_list_.push_back(hash_val);
	}
}

bool ImageProtect::verify_code(verify_code_callback_t cb)
{
	auto NtReadVirtualMemory = IMPORT(L"ntdll.dll", NtReadVirtualMemory);
	std::vector<uint8_t> temp;
	temp.resize(code_size_);
	SIZE_T bytes_of_read = 0;
	auto r = NtReadVirtualMemory(ZwCurrentProcess(), image_base_, temp.data(), code_size_, &bytes_of_read);
	if (!NT_SUCCESS(r))
	{
		return false;
	}

	bool has_difference = false;
	uint8_t* code_buffer = temp.data();
	Utils::Crypto::xor_buffer(back_up_.data(), back_up_.size(), kXorKey);
	for (int n = 0; n < code_size_ / kPageSize; n++)
	{
		uint32_t hash_val = checksum(&code_buffer[n * kPageSize], kPageSize);
		if (hash_val != hash_list_[n])
		{
			for (int i = 0; i < kPageSize; i++)
			{
				if (code_buffer[n * kPageSize + i] != back_up_[n * kPageSize + i])
				{
					if (cb) cb(n * kPageSize + i, sizeof(uint8_t));
					has_difference = true;
				}
			}
		}
	}
	Utils::Crypto::xor_buffer(back_up_.data(), back_up_.size(), kXorKey);
	return has_difference;
}

void ImageProtect::self_remapping()
{
	VMP_VIRTUALIZATION_BEGIN();
	if (is_init())
		return;
	self_remap_is_init_ = true;
	int err_code = 0;
	bool is_packed = true;

#if SUSPEND_OTHER_THREAD
	auto threads = Utils::CWindows::instance().enum_process()[GetCurrentProcessId()].threads;
#endif

	//disable DEP
	ULONG ExecuteFlags = MEM_EXECUTE_OPTION_ENABLE;
	auto NtSetInformationProcess = IMPORT(L"ntdll.dll", NtSetInformationProcess);
	NtSetInformationProcess(
		NtCurrentProcess(),
		ProcessExecuteFlags,
		&ExecuteFlags,
		sizeof(ExecuteFlags));

#if SUSPEND_OTHER_THREAD
	for (auto& thread : threads)
	{
		if (thread.second.tid != GetCurrentThreadId())
		{
			auto thandle = OpenThread(THREAD_ALL_ACCESS, FALSE, thread.second.tid);
			SuspendThread(thandle);
			CloseHandle(thandle);
		}
	}
#endif

	do 
	{
		auto NtCreateSection = IMPORT(L"ntdll.dll", NtCreateSection);
		auto NtMapViewOfSection = IMPORT(L"ntdll.dll", NtMapViewOfSection);
		auto NtUnmapViewOfSection = IMPORT(L"ntdll.dll", NtUnmapViewOfSection);

		void* image = (void*)image_base_;
		auto nt_headers = get_image_nt_header(image);
		auto section_header = image_first_section(nt_headers);

#ifdef _RTEST
		print_pe_section_info(image, nt_headers, section_header);
#endif

		if (*(DWORD*)&section_header[0].Name == 'EDOC')
		{
			code_size_ = section_header[0].Misc.VirtualSize - section_header[0].Misc.VirtualSize % allocation_granularity_;
			is_packed = false;
		}

		LOG_EVENT("is_packed %d code size %08X", is_packed, code_size_);

		image_size_ = nt_headers->OptionalHeader.SizeOfImage;
		NTSTATUS status = STATUS_SUCCESS;
		LARGE_INTEGER section_size = {};

		section_size.QuadPart = image_size_;

		status = NtCreateSection(
			&image_section_handle_,
			SECTION_ALL_ACCESS,
			NULL,
			&section_size,
			PAGE_EXECUTE_READWRITE,
			SEC_COMMIT | SEC_NO_CHANGE,
			NULL);

		if (!NT_SUCCESS(status))
		{
			LOG_EVENT("CreateSection failed");
			err_code = __LINE__;
			self_remap_is_init_ = false;
			break;
		}

		PVOID view_base = map_image();

		if (!view_base)
		{
			LOG_EVENT("view base map failed");
			err_code = __LINE__;
			self_remap_is_init_ = false;
			break;
		}

		LOG_EVENT("view base %08X", view_base);
		LOG_EVENT("wpm %p %p %p ", view_base, image, image_size_);

		SIZE_T bytes_of_write = 0;
		BOOL r = WriteProcessMemory(GetCurrentProcess(), view_base, image, image_size_, &bytes_of_write);
		if (r == FALSE)
		{
			memcpy(view_base, image, image_size_);
			LOG_EVENT("memcpy");
		}

		LOG_EVENT("init checksum");
		back_up_.resize(code_size_);
		memcpy(back_up_.data(), view_base, code_size_);
		init_checksum();
		Utils::Crypto::xor_buffer(back_up_.data(), code_size_, kXorKey);

		if (*(unsigned short*)view_base != IMAGE_DOS_SIGNATURE)
		{
			err_code = __LINE__;
			self_remap_is_init_ = false;
			break;
		}

		NtUnmapViewOfSection(NtCurrentProcess(), image);

		PVOID code_base = (PVOID)image;
		LARGE_INTEGER section_offset = {};
		SIZE_T view_size = code_size_;
		section_offset.HighPart = 0;

		status = NtMapViewOfSection(
			image_section_handle_,
			NtCurrentProcess(),
			&code_base,
			0,
			0,
			&section_offset,
			&view_size,
			ViewUnmap,
			SEC_NO_CHANGE,
			PAGE_EXECUTE_READ);

		if (!NT_SUCCESS(status))
		{
			err_code = __LINE__;
			LOG_EVENT("map view of section failed");
			self_remap_is_init_ = false;
			break;
		}

		section_offset.HighPart = code_size_;
		PVOID data_base = (BYTE*)image + code_size_;
		view_size = image_size_ - code_size_;
		status = NtMapViewOfSection(
			image_section_handle_,
			NtCurrentProcess(),
			&data_base,
			0,
			0,
			&section_offset,
			&view_size,
			ViewUnmap,
			SEC_NO_CHANGE,
			PAGE_EXECUTE_READWRITE);

		if (!NT_SUCCESS(status))
		{
			LPVOID rest = VirtualAlloc((BYTE*)image + code_size_, image_size_ - code_size_, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			LOG_EVENT("%08X", rest);
			if (!rest)
			{
				err_code = __LINE__;
				LOG_EVENT("virtual alloc failed 0x%08X", GetLastError());
				self_remap_is_init_ = false;
				break;
			}

			BYTE* backup_base = (BYTE*)view_base;
			bytes_of_write = 0;
			r = WriteProcessMemory(GetCurrentProcess(), (BYTE*)image + code_size_, backup_base + code_size_, image_size_ - code_size_, &bytes_of_write);
			if (r == FALSE)
			{
				LOG_EVENT("memcpy2");
				memcpy((BYTE*)image + code_size_, backup_base + code_size_, image_size_ - code_size_);
			}
		}

#if RESUME_OTHER_THREAD
		for (auto& thread : threads)
		{
			if (thread.second.tid != GetCurrentThreadId())
			{
				auto thandle = OpenThread(THREAD_ALL_ACCESS, FALSE, thread.second.tid);
				ResumeThread(thandle);
				CloseHandle(thandle);
			}
		}
#endif
		LOG_EVENT("unmap view base");
		unmap_image(view_base);
		LightHook::set_memcpy_func(&image_protect_memcpy);
	} while (0);
	
	if (!self_remap_is_init_)
	{
		char msg[255] = {0};
		snprintf(msg, sizeof(msg) - 1, "进入兼容模式:%d", err_code);
		::MessageBoxA(NULL, msg, "提示", MB_OK);
	}

	LOG_EVENT("cb");
	if (callback_)
		callback_();
	LOG_EVENT("imgp done");
	VMP_VIRTUALIZATION_END();
}

void ImageProtect::register_callback(image_protect_callback_t cb)
{
	callback_ = cb;
}

void ImageProtect::install()
{
	//VMP_VIRTUALIZATION_BEGIN();
	static bool is_remapped = false;

#ifdef _RTEST
	void* image = (void*)image_base_;
	auto nt_headers = GET_IMAGE_NT_HEADER(image);
	auto section_header = IMAGE_FIRST_SECTION(nt_headers);
	print_pe_section_info(image, nt_headers, section_header);
#endif

	ep_hook.install(&GetCommandLineA, [](LightHook::Context& ctx) {
		uint32_t ret_address = *(uint32_t*)ctx.esp;
		void* image = GetModuleHandleA(NULL);
		auto nt_headers = get_image_nt_header(image);
		if (((uint32_t)image < ret_address) &&
			(ret_address < ((uint32_t)image + IMAGE_FIRST_SECTION(nt_headers)->Misc.VirtualSize)))
		{
			LOG_EVENT("%08X", ret_address);
			if (is_remapped == false)
			{
				ImageProtect::instance().self_remapping();
				is_remapped = true;
				ep_hook.restore();
			}
		}
	});
	//VMP_VIRTUALIZATION_END();
}

void ImageProtect::print_pe_section_info(void* image, PIMAGE_NT_HEADERS nt_headers, PIMAGE_SECTION_HEADER section_header)
{
	for (WORD i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i)
	{
		auto section_base =
			nt_headers->OptionalHeader.ImageBase +
			section_header[i].VirtualAddress;

		if (i == 0 && *(DWORD*)&section_header[i].Name == 'EDOC')
		{
			code_size_ = section_header[i].Misc.VirtualSize - section_header[i].Misc.VirtualSize % allocation_granularity_;
			LOG_EVENT("code seg aligned %08X\n", code_size_);
		}

		LOG_EVENT("    %-8.8s    0x%IX - 0x%IX,  0x%08X\n",
			section_header[i].Name,
			section_base,
			section_base + section_header[i].Misc.VirtualSize,
			section_header[i].Misc.VirtualSize);

        if (!is_align(
            section_base,
            allocation_granularity_))
		{
			LOG_EVENT("Unexpected section alignment. (SectionBase = 0x%IX)\n",
				section_base);
		}
	}
	LOG_EVENT("image size %08X", nt_headers->OptionalHeader.SizeOfImage);
}

void* ImageProtect::map_image()
{
	auto NtMapViewOfSection = IMPORT(L"ntdll.dll", NtMapViewOfSection);
	NTSTATUS status = STATUS_SUCCESS;
	PVOID view_base = NULL;
	LARGE_INTEGER section_offset = {};
	SIZE_T view_size = 0;

	status = NtMapViewOfSection(
		image_section_handle_,
		NtCurrentProcess(),
		&view_base,
		0,
		image_size_,
		&section_offset,
		&view_size,
		ViewUnmap,
		0,
		PAGE_READWRITE);

	if (NT_SUCCESS(status))
		return view_base;
	return NULL;
}

void ImageProtect::unmap_image(void* image)
{
	auto NtUnmapViewOfSection = IMPORT(L"ntdll.dll", NtUnmapViewOfSection);
	NtUnmapViewOfSection(NtCurrentProcess(), image);
}

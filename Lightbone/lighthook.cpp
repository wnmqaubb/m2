#include <windows.h>
#include "lighthook.h"
#include "LDasm.h"
#include "utils.h"
using namespace LightHook;

static bool memory_copy(void* dst, void* src, unsigned int size);

MemoryCopyFunction LightHook::memcpy_ = &memory_copy;

static bool memory_copy(void* dst, void* src, unsigned int size)
{
	DWORD oldprotect = 0;
	BOOL status = FALSE;
	status = VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	if (status == FALSE)
		return false;
	if (size == sizeof(uint64_t))
		InterlockedExchange64((LONG64*)dst, *(uint64_t*)src);
	else
		memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
	return true;
}


InlineHook::InlineHook()
{
#ifdef _WIN64
	hook_size_ = 14;
#else
	hook_size_ = 5;
#endif
	src_ = nullptr;
	dst_ = nullptr;
	hooked_ = false;
    is_remote_hook_ = false;
	errno_ = 0;
	memset(origincode_, 0, CODE_CACHE_SIZE);
	memset(trampoline_, 0, CODE_CACHE_SIZE);
	DWORD oldprotect = 0;
	VirtualProtect(trampoline_, CODE_CACHE_SIZE, PAGE_EXECUTE_READWRITE, &oldprotect);
}
InlineHook::~InlineHook()
{
	if (hooked_)
	{
		restore();
	}
}
uint32_t __fastcall InlineHook::get_patch_size(void* ptr)
{
	ldasm_data data = { 0 };
	uint32_t pos = 0;
	uint8_t* code_ptr = reinterpret_cast<uint8_t*>(ptr);

	for (
		 pos = 0;
		 pos < hook_size_;
		 pos += ldasm(&code_ptr[pos], &data, is_x64)
		);
	return pos;
}

bool InlineHook::copy_old_code(void* ptr, uint32_t code_size)
{
	if (code_size > CODE_CACHE_SIZE)
	{
		errno_ = __LINE__;
		return false;
	}
	memcpy(origincode_, ptr, max(code_size,8));
	uint8_t* code_ptr = reinterpret_cast<uint8_t*>(ptr);
	uint32_t pos = 0;
	uint32_t fixed_size = 0;
	for (pos = 0; pos < code_size; )
	{
		ldasm_data ld = { 0 };
		uint32_t len = ldasm(&code_ptr[pos], &ld, is_x64);
		memcpy(&trampoline_[pos + fixed_size], &code_ptr[pos], len);
		if (ld.flags & F_RELATIVE)
		{
			intptr_t distance = 0;
			const uintptr_t ofst = (ld.disp_offset != 0 ? ld.disp_offset : ld.imm_offset);
			const uintptr_t sz = ld.disp_size != 0 ? ld.disp_size : ld.imm_size;

			memcpy(&distance, &code_ptr[pos + ofst], sz);

			uintptr_t ofst_imm = (uintptr_t)(&code_ptr[pos] + len + distance);
#if is_x64 == 1
			// exit if jump is greater then 2GB
			if (_abs64(ofst_imm - (uintptr_t)&trampoline_[pos + fixed_size]) > INT_MAX)
			{
				if (*(uint16_t*)&code_ptr[pos] == 0x3944
					&& code_ptr[pos + 2] == 0x1D)
				{
					/*
					48 B8 imm  -  mov rax,imm
					44 39 18   -  cmp [rax],r11d
					*/
					*(uint16_t*)&trampoline_[pos + fixed_size] = 0xB848;
					*(uint64_t*)&trampoline_[pos + fixed_size + 2] = ofst_imm;
					*(uint16_t*)&trampoline_[pos + fixed_size + 10] = 0x3944;
					trampoline_[pos + fixed_size + 12] = 0x18;
					fixed_size += 6;
				}
				else if (sz == 1)
				{
					uint8_t jmp_code[] = {
							0xFF,0x25,
							0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
					};
					switch (code_ptr[pos])
					{
					case 0x74:
						/*
						75 0E                 - jne here
						FF25 00000000 imm     - jmp imm
						here:
						*/
						*(uint16_t*)& trampoline_[pos + fixed_size] = 0x0E75;
						*(uint64_t*)& jmp_code[6] = ofst_imm;
						memcpy(&trampoline_[pos + fixed_size + 2], jmp_code, sizeof(jmp_code));
						fixed_size += 14;
						break;
					case 0xEB:
						/*
						FF25 00000000 imm     - jmp imm
						here:
						*/
						*(uint64_t*)&jmp_code[6] = ofst_imm;
						memcpy(&trampoline_[pos + fixed_size], jmp_code, sizeof(jmp_code));
						fixed_size += 12;
						break;
					default:
						goto exit;
						break;
					}
					
				}
				else
				{
					exit:
					errno_ = __LINE__;
					return false;
				}
			}
#else
            {
                intptr_t delta = static_cast<intptr_t>(&code_ptr[pos] - &trampoline_[pos + fixed_size]);
				switch (sz)
				{
				case 1:
					distance = delta + (int8_t)distance;
					break;
				case 2:
					distance = delta + (int16_t)distance;
					break;
				case 4:
					distance = delta + (int32_t)distance;
					break;
				case 8:
					distance = delta + (int64_t)distance;
					break;
				default:
					errno_ = __LINE__;
					return false;
				}
                /*if (sz < 4 && distance >> (sz * 8))
                {
                    if (code_ptr[pos] == 0xEB)
                    {
                        trampoline_[pos + fixed_size] = 0xE9;
                        *(uint32_t*)&trampoline_[pos + fixed_size + 1] = (uint32_t)distance - 3;
                        fixed_size += 4;
                    }
                    else
                    {
                        errno_ = __LINE__;
                        return false;
                    }
                }
                else*/
                {
					memcpy(&trampoline_[pos + fixed_size] + ofst, &distance, sz);
				}
			}
#endif		
		}
		pos += len;
	}
#ifdef _WIN64
	uint8_t jmp_code[] = {
		0xFF,0x25,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};
	*(uint64_t*)&jmp_code[6] = reinterpret_cast<uint64_t>(&code_ptr[pos]);
	memcpy(&trampoline_[pos + fixed_size], jmp_code, sizeof(jmp_code));
#else
	trampoline_[pos + fixed_size] = 0xE9;
	*(uint32_t*)&trampoline_[pos + fixed_size + 1] = &code_ptr[pos] - &trampoline_[pos + fixed_size] - 5;
#endif
	return true;

}
bool InlineHook::install(void* src, void* dst, void* trampoline, bool isResolveJmp)
{
	if (src == NULL)
		return false;
    if(isResolveJmp)
    {
        src = ResolveJmp(src);
    }
	uint32_t patch_size = get_patch_size(src);
	if (copy_old_code(src, patch_size) == false)
	{
		errno_ = __LINE__;
		return false;
	}
	if (trampoline)
	{
		*(void**)trampoline = this->trampoline_;
	}
	bool r = false;
#ifdef _WIN64
	uint8_t jmp_code[] = { 
		0xFF,0x25,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};
	*(uint64_t*)&jmp_code[6] = reinterpret_cast<uint64_t>(dst);
	r = memcpy_(src, jmp_code, sizeof(jmp_code));
#else
	int32_t distance = (uint8_t*)dst - (uint8_t*)src - 5;
	uint64_t jmp_code = 0xCCCCCCCCCCCCCCCC;
	memcpy(&jmp_code, src, sizeof(uint64_t));
	uint8_t* jmp_code_ptr = (uint8_t*)&jmp_code;
	jmp_code_ptr[0] = 0xE9;
	*(uint32_t*)&jmp_code_ptr[1] = distance;
	r = memcpy_(src, &jmp_code, sizeof(jmp_code));
#endif
	hooked_ = true;
	src_ = src;
	dst_ = dst;
	return r;
}
void InlineHook::restore()
{
	if (hooked_)
	{
		uint64_t original = *(uint64_t*)origincode_;
		memcpy_(src_, &original, sizeof(original));
		hooked_ = false;
	}
		
}

void LightHook::set_memcpy_func(__in MemoryCopyFunction func)
{
	memcpy_ = func;
}

LightHook::MemoryCopyFunction LightHook::get_origin_memcpy_func()
{
	return &memory_copy;
}

bool LightHook::InlineHook::is_remote_hook() const
{
    return is_remote_hook_;
}

bool LightHook::InlineHook::is_hooked() const
{
    return hooked_;
}

uint32_t LightHook::InlineHook::get_last_error() const
{
    return errno_;
}

void* LightHook::InlineHook::get_hook_addr() const
{
    return src_;
}

void* LightHook::InlineHook::get_dst() const
{
    return dst_;
}

void* LightHook::InlineHook::get_src() const
{
    return src_;
}

const uint8_t context_protect_x86_code[] = { 
	0x60,0x9C,0x81,0xEC,0x80,0x00,0x00,0x00,0x36,0xF3,0x0F,0x7F,0x7C,0x24,0x70,0x36,
	0xF3,0x0F,0x7F,0x74,0x24,0x60,0x36,0xF3,0x0F,0x7F,0x6C,0x24,0x50,0x36,0xF3,0x0F,
	0x7F,0x64,0x24,0x40,0x36,0xF3,0x0F,0x7F,0x5C,0x24,0x30,0x36,0xF3,0x0F,0x7F,0x54,
	0x24,0x20,0x36,0xF3,0x0F,0x7F,0x4C,0x24,0x10,0x36,0xF3,0x0F,0x7F,0x04,0x24,0x68,
	0xCC,0xCC,0xCC,0xCC,0x68,0xEF,0xBE,0xAD,0xDE,0x54,0xB8,0xCC,0xCC,0xCC,0xCC,0xFF,
	0xD0,0x83,0xC4,0x08,0x36,0xF3,0x0F,0x6F,0x04,0x24,0x36,0xF3,0x0F,0x6F,0x4C,0x24,
	0x10,0x36,0xF3,0x0F,0x6F,0x54,0x24,0x20,0x36,0xF3,0x0F,0x6F,0x5C,0x24,0x30,0x36,
	0xF3,0x0F,0x6F,0x64,0x24,0x40,0x36,0xF3,0x0F,0x6F,0x6C,0x24,0x50,0x36,0xF3,0x0F,
	0x6F,0x74,0x24,0x60,0x36,0xF3,0x0F,0x6F,0x7C,0x24,0x70,0x81,0xC4,0x80,0x00,0x00,
	0x00,0x9D,0x61,0x36,0xFF,0xB4,0x24,0x54,0xFF,0xFF,0xFF,0x36,0x81,0x3C,0x24,0xEF,
	0xBE,0xAD,0xDE,0x0F,0x84,0x01,0x00,0x00,0x00,0xC3,0x36,0xC7,0x04,0x24,0xCC,0xCC,
	0xCC,0xCC,0xC3
};

const uint8_t context_protect_x64_code[] = {
	0x54,0x50,0x51,0x52,0x53,0x36,0x48,0xFF,0x74,0x24,0x20,0x55,0x56,0x57,0x41,0x57,
	0x41,0x56,0x41,0x55,0x41,0x54,0x41,0x53,0x41,0x52,0x41,0x51,0x41,0x50,0x9C,0x48,
	0x81,0xEC,0x80,0x00,0x00,0x00,0x36,0xF3,0x0F,0x7F,0x7C,0x24,0x70,0x36,0xF3,0x0F,
	0x7F,0x74,0x24,0x60,0x36,0xF3,0x0F,0x7F,0x6C,0x24,0x50,0x36,0xF3,0x0F,0x7F,0x64,
	0x24,0x40,0x36,0xF3,0x0F,0x7F,0x5C,0x24,0x30,0x36,0xF3,0x0F,0x7F,0x54,0x24,0x20,
	0x36,0xF3,0x0F,0x7F,0x4C,0x24,0x10,0x36,0xF3,0x0F,0x7F,0x04,0x24,0x48,0xB8,0xCC,
	0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x50,0x48,0xB8,0xEF,0xBE,0xAD,0xDE,0xEF,0xBE,
	0xAD,0xDE,0x50,0x48,0x8B,0xCC,0x48,0x83,0xEC,0x20,0x48,0xF7,0xC4,0x08,0x00,0x00,
	0x00,0x74,0x0F,0x90,0x90,0x90,0x90,0x48,0xB8,0xEF,0xBE,0xAD,0xDE,0xEF,0xBE,0xAD,
	0xDE,0x50,0x48,0xB8,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFF,0xD0,0x48,0xB8,
	0xEF,0xBE,0xAD,0xDE,0xEF,0xBE,0xAD,0xDE,0x36,0x48,0x39,0x04,0x24,0x75,0x05,0x90,
	0x90,0x90,0x90,0x58,0x48,0x83,0xC4,0x30,0x36,0xF3,0x0F,0x6F,0x04,0x24,0x36,0xF3,
	0x0F,0x6F,0x4C,0x24,0x10,0x36,0xF3,0x0F,0x6F,0x54,0x24,0x20,0x36,0xF3,0x0F,0x6F,
	0x5C,0x24,0x30,0x36,0xF3,0x0F,0x6F,0x64,0x24,0x40,0x36,0xF3,0x0F,0x6F,0x6C,0x24,
	0x50,0x36,0xF3,0x0F,0x6F,0x74,0x24,0x60,0x36,0xF3,0x0F,0x6F,0x7C,0x24,0x70,0x48,
	0x81,0xC4,0x80,0x00,0x00,0x00,0x9D,0x41,0x58,0x41,0x59,0x41,0x5A,0x41,0x5B,0x41,
	0x5C,0x41,0x5D,0x41,0x5E,0x41,0x5F,0x5F,0x5E,0x5D,0x58,0x5B,0x5A,0x59,0x5C,0x48,
	0x94,0x36,0x48,0xFF,0xB4,0x24,0xE0,0xFE,0xFF,0xFF,0x50,0x48,0xB8,0xEF,0xBE,0xAD,
	0xDE,0xEF,0xBE,0xAD,0xDE,0x36,0x48,0x39,0x44,0x24,0x08,0x58,0x74,0x05,0x90,0x90,
	0x90,0x90,0xC3,0x50,0x48,0xB8,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x36,0x48,
	0x89,0x44,0x24,0x08,0x58,0xC3
};

ContextHook::ContextHook()
{
	memset(context_protect_code, 0, CODE_CACHE_SIZE);
	DWORD oldprotect = 0;
	VirtualProtect(context_protect_code, CODE_CACHE_SIZE, PAGE_EXECUTE_READWRITE, &oldprotect);
}
bool ContextHook::install(void* target, ContextHookHandler handler, void* custom_param)
{
#ifdef _WIN64
	memcpy(context_protect_code, context_protect_x64_code, sizeof(context_protect_x64_code));
	*(uint64_t*)&context_protect_code[0x5F] = (uint64_t)custom_param;
	*(uint64_t*)&context_protect_code[0x6A] = (uint64_t)target;
	*(uint64_t*)&context_protect_code[0x94] = (uint64_t)handler;
	*(uint64_t*)&context_protect_code[0x11D] = (uint64_t)target;
	return InlineHook::install(target, context_protect_code, &context_protect_code[0x136]);
#else
	memcpy(context_protect_code, context_protect_x86_code, sizeof(context_protect_x86_code));
	*(uint32_t*)&context_protect_code[0x40] = (uint32_t)custom_param;
	*(uint32_t*)&context_protect_code[0x45] = (uint32_t)target;
	*(uint32_t*)&context_protect_code[0x4B] = (uint32_t)handler;
	*(uint32_t*)&context_protect_code[0x9F] = (uint32_t)target;

	return InlineHook::install(target, context_protect_code, &context_protect_code[0xAE]);
#endif
	
}

LightHook::ContextHook::~ContextHook()
{
    InlineHook::~InlineHook();
}

LightHook::RemoteHook::RemoteHook(HANDLE process_handle)
{
    process_handle_ = process_handle;
    remote_shellcode_ptr_ = nullptr;
    remote_shellcode_size_ = 0;
}

LightHook::RemoteHook::~RemoteHook()
{

}

bool LightHook::RemoteHook::remote_copy_old_code(uint8_t* ptr, uint32_t code_size)
{
    if (code_size > CODE_CACHE_SIZE)
    {
        errno_ = __LINE__;
        return false;
    }
    SIZE_T bytes_of_read = 0;
    SIZE_T bytes_of_write = 0;

    uint8_t* code_ptr = reinterpret_cast<uint8_t*>(origincode_);
    uint32_t pos = 0;
    uint32_t fixed_size = 0;
    for (pos = 0; pos < code_size; )
    {
        ldasm_data ld = { 0 };
        uint32_t len = ldasm(&code_ptr[pos], &ld, is_x64);
        memcpy(&trampoline_[pos + fixed_size], &code_ptr[pos], len);
        if (ld.flags & F_RELATIVE)
        {
            intptr_t distance = 0;
            const uintptr_t ofst = (ld.disp_offset != 0 ? ld.disp_offset : ld.imm_offset);
            const uintptr_t sz = ld.disp_size != 0 ? ld.disp_size : ld.imm_size;

            memcpy(&distance, &code_ptr[pos + ofst], sz);

            intptr_t ofst_imm = (intptr_t)(&ptr[pos] + len + distance);
            // exit if jump is greater then 2GB
            if (_abs64(ofst_imm - (intptr_t)&remote_trampoline_ptr_[pos + fixed_size]) > INT_MAX)
            {
                errno_ = __LINE__;
                return false;
            }
            else
            {
                intptr_t delta = static_cast<intptr_t>(&ptr[pos] - &remote_trampoline_ptr_[pos + fixed_size]);
                switch (sz)
                {
                case 1:
                    distance = delta + (int8_t)distance;
                    break;
                case 2:
                    distance = delta + (int16_t)distance;
                    break;
                case 4:
                    distance = delta + (int32_t)distance;
                    break;
                case 8:
                    distance = delta + (int64_t)distance;
                    break;
                default:
                    errno_ = __LINE__;
                    return false;
                }
                memcpy(&trampoline_[pos + fixed_size] + ofst, &distance, sz);
            }

        }
        pos += len;
    }

    trampoline_[pos + fixed_size] = 0xE9;
    *(uint32_t*)&trampoline_[pos + fixed_size + 1] = &ptr[pos] - &remote_trampoline_ptr_[pos + fixed_size] - 5;
    return WriteProcessMemory(process_handle_,
        remote_trampoline_ptr_,
        trampoline_,
        remote_trampoline_size_,
        &bytes_of_write);
}

bool LightHook::RemoteHook::install(__in void* src, __in std::string& shellcode, size_t trampoline_ofst)
{
    remote_shellcode_size_ = shellcode.size() < CODE_CACHE_SIZE ? CODE_CACHE_SIZE : shellcode.size();
    remote_shellcode_ptr_ = VirtualAllocEx(process_handle_,
        NULL, 
        remote_shellcode_size_,
        MEM_RESERVE | MEM_COMMIT, 
        PAGE_EXECUTE_READWRITE);
    if (!remote_shellcode_ptr_)
    {
        errno_ = __LINE__;
        return false;
    }
    remote_trampoline_size_ = CODE_CACHE_SIZE;
    remote_trampoline_ptr_ = (uint8_t*)VirtualAllocEx(process_handle_,
        NULL,
        remote_trampoline_size_,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_EXECUTE_READWRITE);
    if (!remote_trampoline_ptr_)
    {
        errno_ = __LINE__;
        return false;
    }
    
    SIZE_T bytes_of_read = 0;
    SIZE_T bytes_of_write = 0;
    if (!ReadProcessMemory(process_handle_, src, origincode_, CODE_CACHE_SIZE, &bytes_of_read))
    {
        errno_ = __LINE__;
        return false;
    }
    patch_size_ = get_patch_size(origincode_);

    if (!remote_copy_old_code((uint8_t*)src, patch_size_))
    {
        VirtualFreeEx(process_handle_, remote_shellcode_ptr_, remote_shellcode_size_, MEM_RELEASE);
        VirtualFreeEx(process_handle_, remote_trampoline_ptr_, remote_trampoline_size_, MEM_RELEASE);
        errno_ = __LINE__;
        return false;
    }

#if 0
    shellcode += "\xE9\xCC\xCC\xCC\xCC";
    int32_t* dist = (int32_t*)(shellcode.data() + shellcode.size() - 4);
    *dist = (int32_t)remote_trampoline_ptr_ - ((int32_t)remote_shellcode_ptr_ + shellcode.size());
#endif

    if (trampoline_ofst)
    {
        *(uint8_t**)(shellcode.data() + trampoline_ofst) = remote_trampoline_ptr_;
    }

    if (!WriteProcessMemory(process_handle_,
        remote_shellcode_ptr_,
        shellcode.data(),
        shellcode.size(),
        &bytes_of_write))
    {
        VirtualFreeEx(process_handle_, remote_shellcode_ptr_, remote_shellcode_size_, MEM_RELEASE);
        VirtualFreeEx(process_handle_, remote_trampoline_ptr_, remote_trampoline_size_, MEM_RELEASE);
        errno_ = __LINE__;
        return false;
    }
    

    uint8_t jmp_code[] = { 0xE9, 0, 0, 0, 0};
    *(int32_t*)(&jmp_code[1]) = (int32_t)remote_shellcode_ptr_ - (int32_t)src - 5;
    DWORD oldprotect;
    VirtualProtectEx(process_handle_, src, sizeof(jmp_code), PAGE_EXECUTE_READWRITE, &oldprotect);
    if (!WriteProcessMemory(process_handle_,
        src,
        jmp_code,
        sizeof(jmp_code),
        &bytes_of_write))
    {
        VirtualFreeEx(process_handle_, remote_shellcode_ptr_, remote_shellcode_size_, MEM_RELEASE);
        VirtualFreeEx(process_handle_, remote_trampoline_ptr_, remote_trampoline_size_, MEM_RELEASE);
        errno_ == __LINE__;
        return false;
    }
    VirtualProtectEx(process_handle_, src, sizeof(jmp_code), oldprotect, &oldprotect);
    return true;
}

void LightHook::RemoteHook::restore()
{

}

LightHook::HookMgr::HookMgr()
{
    errno_ = 0;
}

LightHook::HookMgr::~HookMgr()
{
    restore();
}

LightHook::HookMgr& LightHook::HookMgr::instance()
{
    static HookMgr hookmgr_;
    return hookmgr_;
}

uint32_t LightHook::HookMgr::get_last_error()
{
    return errno_;
}

bool LightHook::HookMgr::add_remote_inline_hook(HANDLE process_handle, __in void* src, __in std::string& shellcode, __in size_t trampoline_ofst)
{
    RemoteHook* hook = new RemoteHook(process_handle);
    if (hook->install(src, shellcode, trampoline_ofst))
    {
        hookbank_.push_back(hook);
        return true;
    }
    errno_ = hook->get_last_error();
    return false;
}


bool LightHook::HookMgr::add_inline_hook(__in void* src, __in void* dst, __out void* trampoline, bool isResolveJmp)
{
    InlineHook* hook = new InlineHook();
    if (hook->install(src, dst, trampoline, isResolveJmp))
    {
        hookbank_.push_back(hook);
        return true;
    }
    errno_ = hook->get_last_error();
	delete hook;
    return false;
}

bool LightHook::HookMgr::add_context_hook(__in void* target, __in ContextHookHandler handler, __in void* custom_param /*= nullptr*/)
{
    ContextHook* hook = new ContextHook();
    if (hook->install(target, handler, custom_param))
    {
        hookbank_.push_back(hook);
        return true;
    }
    errno_ = hook->get_last_error();
	delete hook;
    return false;
}

LightHook::HookMgr::HookBank LightHook::HookMgr::get_hooks() const
{
    return hookbank_;
}

void LightHook::HookMgr::restore()
{
    for (HookBank::iterator itor = hookbank_.begin(); itor != hookbank_.end(); itor++)
    {
        delete* itor;
    }
    hookbank_.clear();
}

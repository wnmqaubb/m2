#pragma once
#include <Windows.h>
#include <stdint.h>
#include <vector>
#define LIGHT_BONE_API __declspec(dllexport)
#define CODE_CACHE_SIZE 0x1000
namespace LightHook
{
	typedef bool(__cdecl* MemoryCopyFunction)(void* dst, void* src, unsigned int size);
	void set_memcpy_func(__in MemoryCopyFunction func);
	MemoryCopyFunction get_origin_memcpy_func();
	extern MemoryCopyFunction memcpy_;

	class InlineHook
	{
    protected:
		uint32_t errno_;
		uint32_t hook_size_;
		uint8_t patch_size_;
		bool hooked_;
		void* src_;
		void* dst_;
		uint8_t origincode_[CODE_CACHE_SIZE];
		uint8_t trampoline_[CODE_CACHE_SIZE];
        bool is_remote_hook_;
	public:
        LIGHT_BONE_API InlineHook();
        LIGHT_BONE_API ~InlineHook();
        LIGHT_BONE_API uint32_t __fastcall get_patch_size(__in void* ptr);
        LIGHT_BONE_API bool copy_old_code(__in void* ptr, __in uint32_t code_size);
		virtual bool install(__in void* src, __in  void* dst, __out void* trampoline, bool isResolveJmp = true);
		virtual void restore();
        LIGHT_BONE_API bool is_remote_hook() const;
        LIGHT_BONE_API bool is_hooked() const;
        LIGHT_BONE_API uint32_t get_last_error() const;
        LIGHT_BONE_API void* get_hook_addr() const;
        LIGHT_BONE_API void* get_dst() const;
        LIGHT_BONE_API void* get_src() const;
	};

    class RemoteHook : public InlineHook
    {
    private:
        HANDLE process_handle_;
        void* remote_shellcode_ptr_;
        size_t remote_shellcode_size_;
        uint8_t* remote_trampoline_ptr_;
        size_t remote_trampoline_size_;
    public:
        LIGHT_BONE_API RemoteHook(HANDLE process_handle);
        LIGHT_BONE_API ~RemoteHook();
        LIGHT_BONE_API bool remote_copy_old_code(uint8_t* ptr, uint32_t code_size);
        virtual bool install(__in void* src, __in std::string& shellcode, size_t trampoline_ofst);
        virtual void restore();
    };

	typedef struct _CONTEXT_X86
	{
		uint32_t eip;
		uint32_t custom_param;
		uint32_t xmm0[4];
		uint32_t xmm1[4];
		uint32_t xmm2[4];
		uint32_t xmm3[4];
		uint32_t xmm4[4];
		uint32_t xmm5[4];
		uint32_t xmm6[4];
		uint32_t xmm7[4];
		uint32_t eflags;
		uint32_t edi;
		uint32_t esi;
		uint32_t ebp;
		uint32_t esp;
		uint32_t ebx;
		uint32_t edx;
		uint32_t ecx;
		uint32_t eax;
	}CONTEXTX86, * LPCONTEXTX86;

	typedef struct _CONTEXT_X64
	{
		uint64_t rip;
		uint64_t custom_param;
		uint32_t xmm0[4];
		uint32_t xmm1[4];
		uint32_t xmm2[4];
		uint32_t xmm3[4];
		uint32_t xmm4[4];
		uint32_t xmm5[4];
		uint32_t xmm6[4];
		uint32_t xmm7[4];
		uint64_t rflags;
		uint64_t r8;
		uint64_t r9;
		uint64_t r10;
		uint64_t r11;
		uint64_t r12;
		uint64_t r13;
		uint64_t r14;
		uint64_t r15;
		uint64_t rdi;
		uint64_t rsi;
		uint64_t rbp;
		uint64_t rsp;
		uint64_t rbx;
		uint64_t rdx;
		uint64_t rcx;
		uint64_t rax;
	}CONTEXTX64, * LPCONTEXTX64;

#ifdef _WIN64
	typedef CONTEXTX64 Context;
#else
	typedef CONTEXTX86 Context;
#endif
    typedef void(__stdcall* ContextHookHandler)(Context& context);
	
	class ContextHook : public InlineHook
	{
	public:
		uint8_t context_protect_code[CODE_CACHE_SIZE];
        LIGHT_BONE_API ContextHook();
        LIGHT_BONE_API ~ContextHook();
		virtual bool install(__in void* target, __in ContextHookHandler handler, __in void* custom_param = nullptr);
	};

	class HookMgr
	{
	private:
		typedef std::vector<InlineHook*> HookBank;
		HookBank hookbank_;
		uint32_t errno_;
	public:
        LIGHT_BONE_API HookMgr();
        LIGHT_BONE_API ~HookMgr();
        LIGHT_BONE_API static HookMgr& instance();
        LIGHT_BONE_API uint32_t get_last_error();
        LIGHT_BONE_API bool add_remote_inline_hook(HANDLE process_handle, __in void* src, __in  std::string& shellcode, __in size_t trampoline_ofst);
        LIGHT_BONE_API bool add_inline_hook(__in void* src, __in  void* dst, __out void* trampoline, bool isResolveJmp = true);
        LIGHT_BONE_API bool add_context_hook(__in void* target, __in ContextHookHandler handler, __in void* custom_param = nullptr);
        LIGHT_BONE_API HookBank get_hooks() const;
        LIGHT_BONE_API void restore();
	};
}



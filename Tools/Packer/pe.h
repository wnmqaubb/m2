#pragma once
#include "coff_reader.h"
#include "lib_reader.h"

template <typename T1, typename T2>
inline T1 cast(T2 v)
{
    return reinterpret_cast<T1>(v);
}
template <typename T>
inline T rva2va(void* image, size_t rva)
{
    return reinterpret_cast<T>(cast<char*>(image) + rva);
}

namespace PE
{
    namespace LdrLoader
    {
        void install_ldr_patch();
        void uninstall_ldr_patch();
        BOOL load_from_buffer(PVOID DllMem, SIZE_T dllSize, LPCWSTR szDllName, HMODULE *pModule);
    }
    namespace PEHandler
    {
        //typedef std::vector<char>(*PayloadGenerator)();
		using payload_generator_t = std::function<std::vector<char>()>;
        bool warp_cert(std::vector<char>& image, PIMAGE_NT_HEADERS nt_header);
        bool inject_payload(std::vector<char>& image, PIMAGE_NT_HEADERS nt_header, payload_generator_t generator);
    }
    typedef std::set<std::string> ModuleList;
	using PEAnalyzerHandler = std::function<bool(std::vector<char>& image, PIMAGE_NT_HEADERS nt_header)>;
	using module_list_t = ModuleList;
	using pe_analyzer_handler_t = PEAnalyzerHandler;

    class PEAnalyzer
    {
    public:
        PEAnalyzer(const std::string& path);
        ~PEAnalyzer() {};
        bool execute_handlers();
        void add_handler(pe_analyzer_handler_t handler);
        int get_imports(module_list_t& out);
        bool apply_change(bool backup = true);
    private:
        bool init_;
        std::ifstream file_;
        std::string path_;
        size_t filesize_;
        std::vector<char> image_;
        PIMAGE_DOS_HEADER dos_header_;
        PIMAGE_NT_HEADERS nt_header_;
        std::vector<PEAnalyzerHandler> handlers_;
    };

}

#endif // _FOUNDATION_PE_H__

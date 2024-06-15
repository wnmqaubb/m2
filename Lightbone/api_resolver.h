<<<<<<< HEAD
#pragma once
#include <stdint.h>
#include <ctype.h>

#define DEFINEAPI(x) decltype(&x) ##x
#define CT_HASH(x) ApiResolver::GetValue<ApiResolver::hash(x)>::value

namespace ApiResolver
{
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

	template <typename T> 
	inline PIMAGE_DOS_HEADER get_image_dos_header(T image)
	{
		static_assert(sizeof(T) == sizeof(void*));
		return reinterpret_cast<PIMAGE_DOS_HEADER>(image);
	}

	template <typename T>
	inline PIMAGE_NT_HEADERS get_image_nt_header(T image)
	{
		static_assert(sizeof(T) == sizeof(void*));
		return ((PIMAGE_NT_HEADERS)((ULONG_PTR)(get_image_dos_header(image)->e_lfanew) + (ULONG_PTR)(image)));
	}
	
	inline PIMAGE_SECTION_HEADER image_first_section(PIMAGE_NT_HEADERS nt_header)
	{
		return IMAGE_FIRST_SECTION(nt_header);
	}

	constexpr inline size_t align_left_size(size_t virtual_size, size_t align_size)
	{
		return (virtual_size%align_size) ? (align_size - (virtual_size%align_size)) : 0;
	}

	constexpr inline size_t align(size_t virtual_size, size_t align_size)
	{
		return virtual_size + align_left_size(virtual_size, align_size);
	}

	inline IMAGE_DATA_DIRECTORY& get_data_directory(PIMAGE_NT_HEADERS nt_header, int dir)
	{
		return nt_header->OptionalHeader.DataDirectory[dir];
	}

    template <typename R = size_t, typename T>
    constexpr inline R fa2ptr(T image, size_t fa)
    {
        static_assert(sizeof(T) == sizeof(void*));
        static_assert(sizeof(R) == sizeof(void*));
        return (R)((size_t)image + fa);
    }

	template <typename R = size_t,typename T>
	constexpr inline R rva2va(T image, size_t rva)
	{
		static_assert(sizeof(T) == sizeof(void*));
		static_assert(sizeof(R) == sizeof(void*));
		return (R)((size_t)image + rva);
	}

    template <typename T>
    inline size_t va2rva(T image, size_t va)
    {
        return va - (size_t)image;
    }

    template <typename T>
    inline size_t rva2fa(T image, size_t rva)
    {
        auto nt_header = get_image_nt_header(image);
        auto sections = image_first_section(nt_header);
        for (int i = 0; i < nt_header->FileHeader.NumberOfSections; i++)
        {
            size_t begin = sections[i].VirtualAddress;
            size_t section_sz = sections[i].SizeOfRawData;
            size_t end = begin + section_sz;
            if (begin <= rva && rva <= end)
            {
                return rva - begin + sections[i].PointerToRawData;
            }
        }
        return 0;
    }

    template<typename T>
    inline size_t va2fa(T image, size_t va)
    {
        return rva2fa(image, va2rva(image, va));
    }

	template <typename T>
	inline size_t get_data_directory_va(T image, int dir)
	{
		return rva2va(image, get_data_directory(get_image_nt_header(image), dir).VirtualAddress);
	}

	template <typename T>
	inline PIMAGE_EXPORT_DIRECTORY get_image_export_directory(T image)
	{
		return (PIMAGE_EXPORT_DIRECTORY)get_data_directory_va(image, IMAGE_DIRECTORY_ENTRY_EXPORT);
	}

	template <typename T>
	inline PIMAGE_IMPORT_DESCRIPTOR get_image_import_directory(T image)
	{
		return (PIMAGE_IMPORT_DESCRIPTOR)get_data_directory_va(image, IMAGE_DIRECTORY_ENTRY_IMPORT);
	}

    template <typename T>
    inline bool is_directory_exist(T image, int dir)
    {
        auto data_dir = get_data_directory(get_image_nt_header(image), dir);
        return data_dir.VirtualAddress && data_dir.Size;
    }

    template <typename T>
    inline bool is_image_import_directory_exist(T image)
    {
        return is_directory_exist(image, IMAGE_DIRECTORY_ENTRY_IMPORT);
    }

	template <typename T>
	inline PIMAGE_TLS_DIRECTORY get_image_tls_directory(T image)
	{
		return (PIMAGE_TLS_DIRECTORY)get_data_directory_va(image, IMAGE_DIRECTORY_ENTRY_TLS);
	}

	typedef struct _TYPEOFFSET
	{
		WORD offset : 12;			//偏移值
		WORD Type : 4;			//重定位属性(方式)
	}TYPEOFFSET, *PTYPEOFFSET;
	
	inline size_t get_reloc_desc_typeoffset_size(PIMAGE_BASE_RELOCATION reloc_desc)
	{
		return ((reloc_desc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(TYPEOFFSET));
	}

    template <typename T>
    inline bool is_align(T ptr, size_t alignment)
    {
        return (((((ULONG_PTR)(ptr)) & (((alignment)-1))) == 0) ? true : false);
    }

    template <typename T>
    inline T rebase(void* ptr, void* origin_base, void* new_base)
    {
        return (T)((uintptr_t)ptr - (uintptr_t)origin_base + (uintptr_t)new_base);
    }
}

=======
#pragma once
#include <stdint.h>
#include <ctype.h>

#define DEFINEAPI(x) decltype(&x) ##x
#define CT_HASH(x) ApiResolver::GetValue<ApiResolver::hash(x)>::value

namespace ApiResolver
{
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

	template <typename T> 
	inline PIMAGE_DOS_HEADER get_image_dos_header(T image)
	{
		static_assert(sizeof(T) == sizeof(void*));
		return reinterpret_cast<PIMAGE_DOS_HEADER>(image);
	}

	template <typename T>
	inline PIMAGE_NT_HEADERS get_image_nt_header(T image)
	{
		static_assert(sizeof(T) == sizeof(void*));
		return ((PIMAGE_NT_HEADERS)((ULONG_PTR)(get_image_dos_header(image)->e_lfanew) + (ULONG_PTR)(image)));
	}
	
	inline PIMAGE_SECTION_HEADER image_first_section(PIMAGE_NT_HEADERS nt_header)
	{
		return IMAGE_FIRST_SECTION(nt_header);
	}

	constexpr inline size_t align_left_size(size_t virtual_size, size_t align_size)
	{
		return (virtual_size%align_size) ? (align_size - (virtual_size%align_size)) : 0;
	}

	constexpr inline size_t align(size_t virtual_size, size_t align_size)
	{
		return virtual_size + align_left_size(virtual_size, align_size);
	}

	inline IMAGE_DATA_DIRECTORY& get_data_directory(PIMAGE_NT_HEADERS nt_header, int dir)
	{
		return nt_header->OptionalHeader.DataDirectory[dir];
	}

    template <typename R = size_t, typename T>
    constexpr inline R fa2ptr(T image, size_t fa)
    {
        static_assert(sizeof(T) == sizeof(void*));
        static_assert(sizeof(R) == sizeof(void*));
        return (R)((size_t)image + fa);
    }

	template <typename R = size_t,typename T>
	constexpr inline R rva2va(T image, size_t rva)
	{
		static_assert(sizeof(T) == sizeof(void*));
		static_assert(sizeof(R) == sizeof(void*));
		return (R)((size_t)image + rva);
	}

    template <typename T>
    inline size_t va2rva(T image, size_t va)
    {
        return va - (size_t)image;
    }

    template <typename T>
    inline size_t rva2fa(T image, size_t rva)
    {
        auto nt_header = get_image_nt_header(image);
        auto sections = image_first_section(nt_header);
        for (int i = 0; i < nt_header->FileHeader.NumberOfSections; i++)
        {
            size_t begin = sections[i].VirtualAddress;
            size_t section_sz = sections[i].SizeOfRawData;
            size_t end = begin + section_sz;
            if (begin <= rva && rva <= end)
            {
                return rva - begin + sections[i].PointerToRawData;
            }
        }
        return 0;
    }

    template<typename T>
    inline size_t va2fa(T image, size_t va)
    {
        return rva2fa(image, va2rva(image, va));
    }

	template <typename T>
	inline size_t get_data_directory_va(T image, int dir)
	{
		return rva2va(image, get_data_directory(get_image_nt_header(image), dir).VirtualAddress);
	}

	template <typename T>
	inline PIMAGE_EXPORT_DIRECTORY get_image_export_directory(T image)
	{
		return (PIMAGE_EXPORT_DIRECTORY)get_data_directory_va(image, IMAGE_DIRECTORY_ENTRY_EXPORT);
	}

	template <typename T>
	inline PIMAGE_IMPORT_DESCRIPTOR get_image_import_directory(T image)
	{
		return (PIMAGE_IMPORT_DESCRIPTOR)get_data_directory_va(image, IMAGE_DIRECTORY_ENTRY_IMPORT);
	}

    template <typename T>
    inline bool is_directory_exist(T image, int dir)
    {
        auto data_dir = get_data_directory(get_image_nt_header(image), dir);
        return data_dir.VirtualAddress && data_dir.Size;
    }

    template <typename T>
    inline bool is_image_import_directory_exist(T image)
    {
        return is_directory_exist(image, IMAGE_DIRECTORY_ENTRY_IMPORT);
    }

	template <typename T>
	inline PIMAGE_TLS_DIRECTORY get_image_tls_directory(T image)
	{
		return (PIMAGE_TLS_DIRECTORY)get_data_directory_va(image, IMAGE_DIRECTORY_ENTRY_TLS);
	}

	typedef struct _TYPEOFFSET
	{
		WORD offset : 12;			//偏移值
		WORD Type : 4;			//重定位属性(方式)
	}TYPEOFFSET, *PTYPEOFFSET;
	
	inline size_t get_reloc_desc_typeoffset_size(PIMAGE_BASE_RELOCATION reloc_desc)
	{
		return ((reloc_desc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(TYPEOFFSET));
	}

    template <typename T>
    inline bool is_align(T ptr, size_t alignment)
    {
        return (((((ULONG_PTR)(ptr)) & (((alignment)-1))) == 0) ? true : false);
    }

    template <typename T>
    inline T rebase(void* ptr, void* origin_base, void* new_base)
    {
        return (T)((uintptr_t)ptr - (uintptr_t)origin_base + (uintptr_t)new_base);
    }
}

>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9

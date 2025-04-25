#include "NewClient/pch.h"
#include <Lightbone/utils.h>
#include "ModuleCheckSum.h"
#include <vector>

using namespace ApiResolver;

CModuleCheckSum::CModuleCheckSum(void* base)
{
	set_init(false);
	set_image_base(base);
	auto nt_header = get_image_nt_header(image_base());
	if (nt_header->FileHeader.NumberOfSections == 0)
		return;
	set_code_size(image_first_section(nt_header)->SizeOfRawData);
	code_ptr(rva2va<char*>(image_base(), image_first_section(nt_header)->VirtualAddress));
    // 添加内存有效性检查
    if (code_ptr() && code_size() > 0 && !IsBadReadPtr(code_ptr(), code_size()))
    {
        //std::copy(code_ptr(), code_ptr() + code_size(), std::back_inserter(code()));
        const bool copyResult = safe_mem_copy(
            reinterpret_cast<std::vector<unsigned char>&>(code()),
            code_ptr(),
            code_size()
        );
        if (!copyResult) {
            set_init(false);
            return;
        }
    }
    else {
        set_init(false);
        return;
    }
	Utils::Crypto::xor_buffer(code().data(), code().size(), 0xABFE2CBE);
	set_hash_val(Utils::Crypto::aphash((const unsigned char*)code_ptr(), code_size()));
	set_init(true);
}

bool CModuleCheckSum::safe_mem_copy(std::vector<unsigned char>& dest, char* src, size_t size) noexcept {
    __try {
        // 添加类型转换
        std::transform(src, src + size, std::back_inserter(dest),
                       [](char c) { return static_cast<unsigned char>(c); });
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool CModuleCheckSum::validate_checksum()
{
    try {
        if (init() == false)
            return true;
        unsigned int cur_hash_val = Utils::Crypto::aphash((const unsigned char*)code_ptr(), code_size());
        return hash_val() == cur_hash_val;
    }
    catch (...)
    {
        return true;
    }
}

void CModuleCheckSum::get_detail(std::function<void(unsigned int, unsigned char, unsigned char)> cb)
{
	std::vector<std::tuple<unsigned int, unsigned char, unsigned char>> result;
	Utils::Crypto::xor_buffer(code().data(), code().size(), 0xABFE2CBE);
	for (unsigned int i = 0; i < code().size(); i++)
	{
		if (code()[i] != (unsigned char)code_ptr()[i])
		{
			cb((unsigned int)&(code_ptr()[i]), code_ptr()[i], code()[i]);
		}
	}
	Utils::Crypto::xor_buffer(code().data(), code().size(), 0xABFE2CBE);
}

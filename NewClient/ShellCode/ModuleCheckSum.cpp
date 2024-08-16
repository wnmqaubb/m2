#include "../pch.h"
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
	std::copy(code_ptr(), code_ptr() + code_size(), std::back_inserter(code()));
	Utils::Crypto::xor_buffer(code().data(), code().size(), 0xABFEBCBE);
	set_hash_val(Utils::Crypto::aphash((const unsigned char*)code_ptr(), code_size()));
	set_init(true);
}

bool CModuleCheckSum::validate_checksum()
{
	if (init() == false)
		return true;
	unsigned int cur_hash_val = Utils::Crypto::aphash((const unsigned char*)code_ptr(), code_size());
	return hash_val() == cur_hash_val;
}

void CModuleCheckSum::get_detail(std::function<void(unsigned int, unsigned char, unsigned char)> cb)
{
	std::vector<std::tuple<unsigned int, unsigned char, unsigned char>> result;
	Utils::Crypto::xor_buffer(code().data(), code().size(), 0xABFEBCBE);
	for (unsigned int i = 0; i < code().size(); i++)
	{
		if (code()[i] != (unsigned char)code_ptr()[i])
		{
			cb((unsigned int)&(code_ptr()[i]), code_ptr()[i], code()[i]);
		}
	}
	Utils::Crypto::xor_buffer(code().data(), code().size(), 0xABFEBCBE);
}
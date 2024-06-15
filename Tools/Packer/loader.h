#pragma once
#include <stdint.h>
#pragma pack(push)
#pragma pack(1)
const uint32_t kShareDateMagicKey = 0xCAFEBABE;
typedef struct _ShareData
{
    uint32_t magic;
    uint8_t  eip_opcode[5];
    uint8_t  call_opcode;
    uint32_t entry_point_dist;
    uint8_t  ret_opcode;
    uint32_t oep;
    uint32_t stub_rva;
    uint32_t stub_size;
    uint32_t stub_entry_rva;
    uint32_t crypt_code_rva;
    uint32_t crypt_code_size;
    uint32_t xor_key;
    uint32_t current_section_rva;
    uint32_t current_section_virtual_size;
    uint32_t origin_image_base;
    uint8_t  stage;
    uint16_t  cfg_size;
    uint8_t  cfg[1000];
}ShareData, *ShareDataPtr;

using share_data_t = ShareData;
using share_data_ptr_t = ShareDataPtr;

typedef struct _RelocInfo
{
    uint32_t type;
    uint32_t rva;
}RelocInfo, *RelocInfoPtr;

using reloc_info_t = RelocInfo;
using reloc_info_ptr_t = RelocInfoPtr;

typedef struct _CryptCode
{
    size_t text_code_size;
    size_t bind_dll_size;
    size_t reloc_size;
    reloc_info_t reloc[1];
}CryptCode, *CryptCodePtr;

using crypt_code_t = CryptCode;
using crypt_code_ptr_t = CryptCodePtr;

extern void __stdcall client_entry(share_data_ptr_t param) noexcept;
#pragma pack(pop)
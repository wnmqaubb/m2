#ifndef UTILS_CODE_VERIFY_H_
#define UTILS_CODE_VERIFY_H_
#include <stdint.h>
#include <vector>
using byte_vector_t = std::vector<unsigned char>;
using uint32_vector_t = std::vector<unsigned int>;

enum PEErrorCode
{
	kSuccess = 0,
	kImageNotReadable,
	kSectionNumZero,
	kSectionVirtualAddressInvalid,
	kSectionNotAligned,
	kReadMemoryError,
	kReadMemorySizeNotMatch,
};

struct SectionInformation
{
	uintptr_t virtual_address;
	unsigned int size;
	uint32_t hash;
	uint32_vector_t hash_detail;
	byte_vector_t buffer;
};

class CodeVerify
{
public:
	CodeVerify();
	void init_module(void* module_base);
	PEErrorCode init_section(void* virtual_address, unsigned int size);
	PEErrorCode verify_section();
	uint32_t checksum(unsigned char* buffer, unsigned int size);
	bool is_ptr_valid(void* va, unsigned int sz);
	bool read_memory(void* va, unsigned int sz, byte_vector_t& out);
	void set_module_base(void* module_base) { module_base_ = module_base; }
	void* get_module_base() { return module_base_; }
	void set_error_code(PEErrorCode code) { error_code_ = code; }
	unsigned int get_limited_verify_size() const { return limited_verify_size_; }
	void set_limited_verify_size(unsigned int size) { limited_verify_size_ = size; }
	PEErrorCode get_error_code() const { return error_code_; }
	SectionInformation& section_information() { return section_information_; }
private:
	void* module_base_;
	unsigned int limited_verify_size_;
	PEErrorCode error_code_;
	SectionInformation section_information_;
    const uint32_t kXorKey = 0x20240723;
    const uint32_t kPageSize = 0x1000;
};

#endif

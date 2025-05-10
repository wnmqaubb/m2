#pragma once

class CModuleCheckSum
{
public:
	CModuleCheckSum() = delete;
	CModuleCheckSum(void* base);
    bool safe_mem_copy(std::vector<unsigned char>& dest, char* src, size_t size) noexcept;
	bool validate_checksum();
	void get_detail(std::function<void(unsigned int, unsigned char, unsigned char)> cb);
public:
	void* image_base() const { return image_base_; }
	void set_image_base(void* val) { image_base_ = val; }
	size_t image_size() const { return image_size_; }
	void set_image_size(size_t val) { image_size_ = val; }
	bool init() const { return init_; }
	void set_init(bool val) { init_ = val; }
	char* code_ptr() const { return code_ptr_; }
	void code_ptr(char* val) { code_ptr_ = val; }
	size_t code_size() const { return code_size_; }
	void set_code_size(size_t val) { code_size_ = val; }
	std::vector<unsigned char>& code() { return code_; }
	size_t hash_val() const { return hash_val_; }
	void set_hash_val(size_t val) { hash_val_ = val; }
private:
	bool init_;
	size_t image_size_;
	void* image_base_;
	char* code_ptr_;
	size_t code_size_;
	unsigned int hash_val_;
	std::vector<unsigned char> code_;
};
#include "lib_reader.h"

using namespace PE;

LibReader::LibReader(const std::string& path)
{
	file_.open(path, std::ios::in | std::ios::binary);
	assert(file_.is_open());
	file_.seekg(0, file_.end);
	file_size_ = file_.tellg();
	headers_ = get_header();
}

coff_reader_shared_ptr_t LibReader::get_object_coff(image_archive_member_header_pair_t& header)
{
	file_.clear();
	file_.seekg(header.second, std::ios::beg);
	std::string obj;
	std::size_t sz = atoi((char*)header.first.Size);
	obj.resize(sz);
	file_.read((char*)obj.data(), sz);
	std::stringstream ss(obj);
	auto coff = std::make_shared<coff_reader_t>(std::move(ss));
	if (coff->string_table_size_ >= coff->file_size_)
	{
		return nullptr;
	}
	return std::move(coff);
}

image_archive_member_header_pair_t LibReader::get_first_object_section()
{
	image_archive_member_header_t null_header = { 0 };
	image_archive_member_header_pair_t null_header_pair = std::make_pair(null_header, 0);
	for (auto& header : headers_)
	{
		std::string name((char*)header.first.Name);
		if (name != IMAGE_ARCHIVE_LINKER_MEMBER && name != IMAGE_ARCHIVE_LONGNAMES_MEMBER)
		{
			return header;
		}
	}
	__debugbreak();
	return null_header_pair;
}

image_archive_member_header_vector_t& LibReader::get_header()
{
	if (headers_.size()) return headers_;
	file_.seekg(0);
	std::string archive_start = IMAGE_ARCHIVE_START;
	file_.read((char*)archive_start.data(), IMAGE_ARCHIVE_START_SIZE);
	assert(archive_start == IMAGE_ARCHIVE_START);
	do
	{
		image_archive_member_header_t img_arch_mem_header = { 0 };
		char sig = 0;
		file_.read(&sig, 1);
		if (sig != '\n')
		{
			file_.seekg(-1, std::ios::cur);
		}
		file_.read((char*)&img_arch_mem_header, sizeof(img_arch_mem_header));
		std::size_t sz = atoi((char*)img_arch_mem_header.Size);
		if (sz == 0)
			break;
		headers_.push_back(std::make_pair(img_arch_mem_header, file_.tellg()));
		std::size_t cur = file_.tellg();
		file_.seekg(sz, std::ios::cur);
	} while (!file_.eof());
	return headers_;
}
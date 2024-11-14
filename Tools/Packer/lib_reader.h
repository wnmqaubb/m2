#pragma once

#include "coff_reader.h"

namespace PE
{
    class LibReader;
    using lib_reader_t = LibReader;
    using image_archive_member_header_t = IMAGE_ARCHIVE_MEMBER_HEADER;
    using image_archive_member_header_pair_t = std::pair<IMAGE_ARCHIVE_MEMBER_HEADER, std::size_t>;
    using image_archive_member_header_vector_t = std::vector<image_archive_member_header_pair_t>;
    using coff_reader_shared_ptr_t = std::shared_ptr<coff_reader_t>;
    class LibReader
    {
    public:
        LibReader(const std::string& path);
        coff_reader_shared_ptr_t get_object_coff(image_archive_member_header_pair_t& header);
        image_archive_member_header_pair_t get_first_object_section();
        ~LibReader() {};

        image_archive_member_header_vector_t& get_header();

        std::ifstream file_;
        size_t file_size_;
        image_archive_member_header_vector_t headers_;
    };
}
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <assert.h>
#include <functional>

namespace PE
{
    union CoffHeader;
    struct FullSymbol;
    struct LinkSection;
    class CoffReader;

    using SectionMap = std::map<int, LinkSection>;
    using SectionVector = std::vector<IMAGE_SECTION_HEADER>;
    using SymbolVector = std::vector<IMAGE_SYMBOL>;
    using StringVector = std::vector<char>;
    using FullSymbolVector = std::vector<FullSymbol>;
    using FullSymbolSet = std::set<FullSymbol>;
    using RelocVector = std::vector<IMAGE_RELOCATION>;

    using coff_reader_t = PE::CoffReader;
    using full_symbol_t = PE::FullSymbol;
    using full_symbol_set_t = FullSymbolSet;
    using full_symbol_vector_t = FullSymbolVector;
	using section_t = IMAGE_SECTION_HEADER;
    using section_vector_t = SectionVector;
    using section_map_t = SectionMap;
    using symbol_vector_t = SymbolVector;
    using string_vector_t = StringVector;
    using image_section_header_t = IMAGE_SECTION_HEADER;
    using image_file_header_t = IMAGE_FILE_HEADER;
    using reloc_vector_t = RelocVector;

    union CoffHeader
    {
        constexpr static const uint8_t bigobj_classid[16] = {
            0xc7, 0xa1, 0xba, 0xd1, 0xee, 0xba, 0xa9, 0x4b,
            0xaf, 0x20, 0xfa, 0xf6, 0x6a, 0xa4, 0xdc, 0xb8,
        };

        //This is actually a 16byte UUID
        static_assert(sizeof(bigobj_classid) == sizeof(CLSID));

        bool is_extended() const {
            return bigobj.Sig1 == 0x0000 && bigobj.Sig2 == 0xFFFF && memcmp(&bigobj.ClassID, bigobj_classid, sizeof(CLSID)) == 0;
        }

        IMAGE_FILE_HEADER obj;
        ANON_OBJECT_HEADER_BIGOBJ bigobj;
    };

    struct FullSymbol
    {
        std::string name;
        DWORD   value;
        SHORT   section_number;
        WORD    type;
        BYTE    storage_class;
        BYTE    number_of_aux_symbols;
        bool operator == (const FullSymbol& o) const
        {
            return o.name == name;
        }
        bool operator < (const FullSymbol& o) const
        {
            return o.name < name;
        }
        bool is_function() const
        {
            return ISFCN(this->type);
        }
        bool is_array() const
        {
            return ISARY(this->type);
        }
        bool is_pointer() const
        {
            return ISPTR(this->type);
        }
        bool is_tag() const
        {
            return ISTAG(this->type);
        }
        bool is_import_symbol() const
        {
            // 如果section编号为未定义
            if (this->section_number == IMAGE_SYM_UNDEFINED)
            {
                // 如果存储类别为外部或弱外部
                if (this->storage_class == IMAGE_SYM_CLASS_EXTERNAL || this->storage_class == IMAGE_SYM_CLASS_WEAK_EXTERNAL)
                {
                    return true;
                }
            }
            return false;
        }
    };
    struct LinkSection
    {
        size_t offset;
        IMAGE_SECTION_HEADER section;
        LinkSection()
        {
            __debugbreak();
        }
        LinkSection(size_t offset_, IMAGE_SECTION_HEADER* section_)
        {
            offset = offset_;
            memcpy(&section, section_, sizeof(IMAGE_SECTION_HEADER));
        }
    };

    class CoffReader
    {
    public:
        CoffReader(const std::string& path);
		void init();
		CoffReader(std::stringstream ss);
		CoffReader(std::istream& istream);
        ~CoffReader() {};
            
        section_vector_t& get_section_header();
        full_symbol_vector_t& get_symbols();
        string_vector_t& get_strings();
        image_section_header_t& get_section_from_symbol(full_symbol_t& symbol);
        std::string get_section_raw_data(image_section_header_t& section);
        reloc_vector_t get_symbol_reloc(full_symbol_t& symbol, const size_t symbol_size);
        reloc_vector_t get_section_reloc(image_section_header_t& section);
        full_symbol_t find_symbol(std::string symbol_name);
		bool find_section(std::string section_text, section_t& out);
		bool find_symbol(std::string symbol_name, full_symbol_t& out);
		size_t get_symbol_raw_data_offset(full_symbol_t& symbol, full_symbol_set_t& symbol_set);
        void fix_reloc(int16_t reloc_type, uint8_t* relocation, uintptr_t relocation_rva, uintptr_t target_rva, uintptr_t image_base);
        bool get_symbol_link_func(std::string symbol_name, full_symbol_set_t& link);
        size_t get_symbol_size(full_symbol_t& symbol);
		inline std::string get_symbol_raw_data(full_symbol_t& symbol)
		{
			return get_section_raw_data(get_section_from_symbol(symbol)).substr(symbol.value, get_symbol_size(symbol));
		}
		std::istream& istream_;
		std::ifstream file_;
		std::stringstream ss_;
		size_t file_size_;
		size_t string_table_size_;
		image_file_header_t header_;
		section_vector_t sections_;
		section_map_t link_sections_;
		full_symbol_vector_t symbols_;
		string_vector_t strings_;
    };
}
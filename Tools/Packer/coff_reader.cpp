#include "coff_reader.h"
#include <istream>

using namespace PE;

bool CoffReader::get_symbol_link_func(std::string symbol_name, full_symbol_set_t& link)
{
    static std::set<std::string> symbol_set;
	full_symbol_t symbol;
	if (!find_symbol(symbol_name, symbol))
	{
		return false;
	}
    if (symbol.is_import_symbol())
    {
        link.emplace(symbol);
        return false;
    }
    if (link.find(symbol) != link.end())
    {
        return false;
    }
    link.emplace(symbol);
    for (auto& reloc : get_symbol_reloc(symbol, get_symbol_size(symbol)))
    {
        full_symbol_t link_func = get_symbols()[reloc.SymbolTableIndex];
        get_symbol_link_func(link_func.name, link);
    }
    return true;
}
//************************************
// Method:    get_symbol_size
// FullName:  CoffReader::get_symbol_size
// Access:    public 
// Returns:   size_t
// Qualifier:
// Parameter: full_symbol_t & symbol
//************************************
size_t CoffReader::get_symbol_size(full_symbol_t& symbol)
{
    if (symbol.is_import_symbol())
    {
        return 0;
    }
    full_symbol_vector_t same_section_funcs;
    int index_in_same_section_funcs = 0;
    std::for_each(get_symbols().begin(), get_symbols().end(), [&](const full_symbol_t& s) {
        if (symbol.section_number == s.section_number)
        {
            same_section_funcs.push_back(s);
            if (symbol == s)
            {
                index_in_same_section_funcs = same_section_funcs.size() - 1;
            }
        }
    });

    if ((index_in_same_section_funcs == same_section_funcs.size() - 1)
        ||
		(same_section_funcs[index_in_same_section_funcs + 1].value == 0 &&
			same_section_funcs[index_in_same_section_funcs].value == 0))
    {
        return get_section_from_symbol(symbol).SizeOfRawData;
    }

    return same_section_funcs[index_in_same_section_funcs + 1].value - same_section_funcs[index_in_same_section_funcs].value;
}

//************************************
// Method:    get_symbol_raw_data_offset
// FullName:  CoffReader::get_symbol_raw_data_offset
// Access:    public 
// Returns:   size_t
// Qualifier:
// Parameter: full_symbol_t & symbol
// Parameter: full_symbol_set_t & symbol_set
//************************************
size_t CoffReader::get_symbol_raw_data_offset(full_symbol_t& symbol, full_symbol_set_t& symbol_set)
{
    auto target_func = symbol_set.find(symbol);
    assert(target_func != symbol_set.end());
    size_t offset = 0;
    bool found = false;
    if (symbol.name == "??_7exception@std@@6B@")
    {
        auto x = get_section_from_symbol(symbol);
        __debugbreak();
    }
    for (full_symbol_set_t::iterator itor = symbol_set.begin();
        itor != symbol_set.end();
        itor++)
    {
        full_symbol_t cur = *itor;
        if (cur.section_number == target_func->section_number
            && cur.value == 0)
        {
            found = true;
            break;
        }
        else if (cur.is_import_symbol() == false && ISFCN(cur.type))
        {
            offset += get_section_from_symbol(cur).SizeOfRawData;
        }
    }
    assert(found);
    offset += target_func->value;
    return offset;
}

//************************************
// Method:    fix_reloc
// FullName:  CoffReader::fix_reloc
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int16_t reloc_type
// Parameter: uint8_t * relocation
// Parameter: uintptr_t relocation_rva
// Parameter: uintptr_t target_rva
// Parameter: uintptr_t image_base
//************************************
void CoffReader::fix_reloc(int16_t reloc_type, uint8_t* relocation, uintptr_t relocation_rva, uintptr_t target_rva, uintptr_t image_base)
{
    switch (reloc_type)
    {
        // No relocation necessary
    case IMAGE_REL_I386_ABSOLUTE:
        __debugbreak();
        break;
        // Absolute virtual address
    case IMAGE_REL_I386_DIR32:
        *reinterpret_cast<uint32_t*>(relocation) = target_rva;
        break;
        // Relative virtual address to __ImageBase
    case IMAGE_REL_I386_DIR32NB:
        *reinterpret_cast<int32_t*>(relocation) = target_rva - image_base;
        break;
        // Relative to next instruction after relocation
    case IMAGE_REL_AMD64_REL32:
    case IMAGE_REL_I386_REL32:
        *reinterpret_cast<int32_t*>(relocation) = target_rva - (relocation_rva + 4);
        break;
    case IMAGE_REL_I386_SECREL:
        *reinterpret_cast<uint32_t *>(relocation) = target_rva & 0xFFF; // TODO: This was found by comparing generated ASM, probably not correct
        break;
    }

}

//************************************
// Method:    CoffReader
// FullName:  CoffReader::CoffReader
// Access:    public 
// Returns:   
// Qualifier:
// Parameter: const std::string & path
//************************************
CoffReader::CoffReader(const std::string& path)
	: file_(path, std::ios::in | std::ios::binary),
	 istream_(file_)
{
	init();
}

CoffReader::CoffReader(std::istream& istream)
	: istream_(istream)
{
	init();
}

CoffReader::CoffReader(std::stringstream ss)
	: ss_(std::move(ss)),
	  istream_(ss_)
{
	init();
}

void CoffReader::init()
{
	istream_.seekg(0, istream_.end);
	file_size_ = istream_.tellg();
	istream_.seekg(0);
	CoffHeader header;
	istream_.read((char*)&header, sizeof(header));
	assert(!header.is_extended());
    if (header.is_extended())
        __debugbreak();
	header_ = header.obj;
	string_table_size_ = file_size_ - (header_.PointerToSymbolTable + header_.NumberOfSymbols * sizeof(*((symbol_vector_t*)0)->data()));

	if (string_table_size_ > file_size_)
		return;

	get_section_header();
	get_strings();
	get_symbols();
}
//************************************
// Method:    get_section_header
// FullName:  CoffReader::get_section_header
// Access:    public 
// Returns:   Gin::PE::section_vector_t&
// Qualifier:
//************************************
section_vector_t& CoffReader::get_section_header()
{
    if (sections_.size())
    {
        return sections_;
    }
    section_vector_t sections(header_.NumberOfSections);
    istream_.seekg(sizeof(header_));
    istream_.read((char*)sections.data(), sizeof(IMAGE_SECTION_HEADER)*header_.NumberOfSections);
    sections_ = sections;
    return sections_;
}

//************************************
// Method:    get_symbols
// FullName:  CoffReader::get_symbols
// Access:    public 
// Returns:   Gin::PE::full_symbol_vector_t&
// Qualifier:
//************************************
full_symbol_vector_t& CoffReader::get_symbols()
{
    if (symbols_.size())
    {
        return symbols_;
    }
    istream_.seekg(header_.PointerToSymbolTable);
    SymbolVector symbols(header_.NumberOfSymbols);
    istream_.read((char*)symbols.data(), header_.NumberOfSymbols * sizeof(*symbols.data()));
    for (auto &symbol : symbols)
    {
        BYTE *target_address = nullptr;
        // Get symbol name from string table if it is a long name
        std::string symbol_name;
        if (symbol.N.Name.Short == 0)
        {
            assert(symbol.N.Name.Long < string_table_size_);
            symbol_name = get_strings().data() + symbol.N.Name.Long;
        }
        else
        {
            const char* short_name = reinterpret_cast<const char*>(symbol.N.ShortName);
            symbol_name = std::string(short_name, strnlen(short_name, IMAGE_SIZEOF_SHORT_NAME));
        }
        full_symbol_t full_symbol;
        full_symbol.name = symbol_name;
        full_symbol.value = symbol.Value;
        full_symbol.type = symbol.Type;
        full_symbol.storage_class = symbol.StorageClass;
        full_symbol.section_number = symbol.SectionNumber;
        full_symbol.number_of_aux_symbols = symbol.NumberOfAuxSymbols;
        symbols_.push_back(full_symbol);
    }
    return symbols_;
}

//************************************
// Method:    get_strings
// FullName:  CoffReader::get_strings
// Access:    public 
// Returns:   Gin::PE::string_vector_t&
// Qualifier:
//************************************
string_vector_t& CoffReader::get_strings()
{
    if (strings_.size())
    {
        return strings_;
    }
    istream_.seekg(header_.PointerToSymbolTable + header_.NumberOfSymbols * sizeof(*((symbol_vector_t*)0)->data()));
    string_vector_t strings(string_table_size_);
    istream_.read(strings.data(), string_table_size_);
    strings_ = strings;
    return strings_;
}

//************************************
// Method:    get_section_from_symbol
// FullName:  CoffReader::get_section_from_symbol
// Access:    public 
// Returns:   Gin::PE::image_section_header_t&
// Qualifier:
// Parameter: full_symbol_t & symbol
//************************************
image_section_header_t& CoffReader::get_section_from_symbol(full_symbol_t& symbol)
{
    return sections_[symbol.section_number - 1];
}

//************************************
// Method:    get_section_raw_data
// FullName:  CoffReader::get_section_raw_data
// Access:    public 
// Returns:   std::string
// Qualifier:
// Parameter: image_section_header_t & section
//************************************
std::string CoffReader::get_section_raw_data(image_section_header_t& section)
{
    std::string raw_data;
    istream_.seekg(section.PointerToRawData);
    raw_data.resize(section.SizeOfRawData);
    istream_.read((char*)raw_data.data(), section.SizeOfRawData);
    return std::move(raw_data);
}
//************************************
// Method:    get_symbol_reloc
// FullName:  CoffReader::get_symbol_reloc
// Access:    public 
// Returns:   Gin::PE::reloc_vector_t
// Qualifier:
// Parameter: full_symbol_t & symbol
// Parameter: const size_t symbol_size
//************************************
reloc_vector_t CoffReader::get_symbol_reloc(full_symbol_t& symbol, const size_t symbol_size)
{
    reloc_vector_t result;
    if (symbol_size)
    {
        for (auto& reloc : get_section_reloc(get_section_from_symbol(symbol)))
        {
            if (symbol.value < reloc.VirtualAddress && reloc.VirtualAddress < (symbol.value + symbol_size))
            {
                result.push_back(reloc);
            }
        }
        return result;
    }
    return get_section_reloc(get_section_from_symbol(symbol));
}
//************************************
// Method:    get_section_reloc
// FullName:  CoffReader::get_section_reloc
// Access:    public 
// Returns:   Gin::PE::reloc_vector_t
// Qualifier:
// Parameter: image_section_header_t & section
//************************************
reloc_vector_t CoffReader::get_section_reloc(image_section_header_t& section)
{
    reloc_vector_t relocs(section.NumberOfRelocations);
    istream_.seekg(section.PointerToRelocations);
    istream_.read((char*)relocs.data(), section.NumberOfRelocations * sizeof(*relocs.data()));
    return std::move(relocs);
}

//************************************
// Method:    find_symbol
// FullName:  CoffReader::find_symbol
// Access:    public 
// Returns:   Gin::PE::full_symbol_t
// Qualifier:
// Parameter: std::string symbol_name
//************************************
full_symbol_t CoffReader::find_symbol(std::string symbol_name)
{
	full_symbol_t symbol;
	bool r = find_symbol(symbol_name, symbol);
	assert(r);
	return symbol;
}

//************************************
// Method:    find_symbol
// FullName:  Gin::PE::CoffReader::find_symbol
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: std::string symbol_name
// Parameter: full_symbol_t & out
//************************************
bool CoffReader::find_symbol(std::string symbol_name, full_symbol_t& out)
{
	for (auto& symbol : get_symbols())
	{
		if (symbol.name == symbol_name)
		{
			out = symbol;
			return true;
		}
	}
	out = full_symbol_t();
	return false;
}

bool CoffReader::find_section(std::string section_name, section_t& out)
{
	for (auto& section : sections_)
	{
		if ((char*)section.Name == section_name)
		{
			out = section;
			return true;
		}
	}
	out = section_t();
	return false;
}
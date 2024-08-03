#include "memory.h"

#define PAGE_SHIFT              (12)
#define PAGE_SIZE 0x1000
#define BYTES_TO_PAGES(Size)    (((Size) >> PAGE_SHIFT) + (((Size) & (PAGE_SIZE - 1)) != 0))
#define ROUND_TO_PAGES(Size)    (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))


GameMemory::GameMemory()
{
    current_process_handle_ = ::GetCurrentProcess();
}

GameMemory::~GameMemory()
{

}

GameMemory& GameMemory::instance()
{
    static GameMemory instance_;
    return instance_;
}

bool GameMemory::memory_read_safe(HANDLE hprocess, LPVOID lp_base_address, LPVOID lpbuffer, SIZE_T nsize, SIZE_T * lp_number_of_bytes_read)
{
    SIZE_T number_of_bytes_read = 0;
    SIZE_T * pnum_bytes = 0;
    DWORD dwprotect = 0;
    bool ret_value = false;

    //read memory
    if((hprocess == 0) || (lp_base_address == 0) || (lpbuffer == 0) || (nsize == 0))
    {
        return false;
    }

    if(!lp_number_of_bytes_read)
    {
        pnum_bytes = &number_of_bytes_read;
    }
    else
    {
        pnum_bytes = lp_number_of_bytes_read;
    }

    if(!ReadProcessMemory(hprocess, lp_base_address, lpbuffer, nsize, pnum_bytes))
    {
        if(VirtualProtectEx(hprocess, lp_base_address, nsize, PAGE_EXECUTE_READWRITE, &dwprotect))
        {
            if(ReadProcessMemory(hprocess, lp_base_address, lpbuffer, nsize, pnum_bytes))
            {
                ret_value = true;
            }
            VirtualProtectEx(hprocess, lp_base_address, nsize, dwprotect, &dwprotect);
        }
    }
    else
    {
        ret_value = true;
    }

    return ret_value;
}

bool GameMemory::mem_read(uint32_t base_address, void* buffer, uint32_t size, uint32_t* number_of_bytes_read)
{
    // Buffer must be supplied and size must be greater than 0
    if(!buffer || size <= 0)
        return false;

    // If the 'bytes read' parameter is null, use a temp
    SIZE_T bytes_read_temp = 0;

    if(!number_of_bytes_read)
        number_of_bytes_read = (uint32_t*)&bytes_read_temp;
    // Normal single-call read
    bool ret = memory_read_safe(current_process_handle_, (LPVOID)base_address, buffer, size, (SIZE_T *)number_of_bytes_read);

    if(ret && *number_of_bytes_read == size)
        return true;

    // Read page-by-page (Skip if only 1 page exists)
    // If (SIZE > PAGE_SIZE) or (ADDRESS exceeds boundary), multiple reads will be needed
    SIZE_T page_count = BYTES_TO_PAGES(size);

    if(page_count > 1)
    {
        // Determine the number of bytes between ADDRESS and the next page
        uint32_t offset = 0;
        uint32_t read_base = base_address;
        uint32_t read_size = ROUND_TO_PAGES(read_base) - read_base;

        // Reset the bytes read count
        *number_of_bytes_read = 0;

        for(SIZE_T i = 0; i < page_count; i++)
        {
            SIZE_T bytes_read = 0;

            if(memory_read_safe(current_process_handle_, (PVOID)read_base, ((PBYTE)buffer + offset), read_size, &bytes_read))
                *number_of_bytes_read += bytes_read;

            offset += read_size;
            read_base += read_size;

            size -= read_size;
            read_size = min(PAGE_SIZE, size);
        }
    }

    SetLastError(ERROR_PARTIAL_COPY);
    return (*number_of_bytes_read > 0);
}

bool GameMemory::mem_isvalid_readptr(uint32_t address)
{
    unsigned char a = 0;
    return mem_read(address, &a, sizeof(unsigned char), nullptr);
}

void GameMemory::mem_update_map()
{
    // First gather all possible pages in the memory range
    std::vector<MEMORY_BASIC_INFORMATION> page_vector;

    static ULONG64 lastProcessesTickCount = 0;
    ULONG64 tickCount;
    tickCount = GetTickCount64();
    if(tickCount - lastProcessesTickCount >= 3000)
    {

        SIZE_T num_bytes = 0;
        uint32_t page_start = 0;
        uint32_t allocation_base = 0;

        do
        {
            // Query memory attributes
            MEMORY_BASIC_INFORMATION mbi;
            memset(&mbi, 0, sizeof(mbi));

            num_bytes = VirtualQueryEx(current_process_handle_, (LPVOID)page_start, &mbi, sizeof(mbi));

            // Only allow pages that are committed to memory (exclude reserved/mapped)
            if(mbi.State == MEM_COMMIT)
            {
                // Only list allocation bases, unless if forced to list all
                if(allocation_base != (uint32_t)mbi.AllocationBase)
                {
                    // Set the new allocation base page
                    allocation_base = (uint32_t)mbi.AllocationBase;

                    page_vector.push_back(mbi);
                }
                else
                {
                    // Otherwise append the page to the last created entry
                    page_vector.back().RegionSize += mbi.RegionSize;
                }
            }

            // Calculate the next page start
            uint32_t new_address = (uint32_t)mbi.BaseAddress + mbi.RegionSize;

            if(new_address <= page_start)
                break;

            page_start = new_address;
        }
        while(num_bytes);

        std::lock_guard<std::mutex> lock(mtx_);
        memory_pages_.clear();

        for(auto & page : page_vector)
        {
            uint32_t start = (uint32_t)page.BaseAddress;
            uint32_t size = (uint32_t)page.RegionSize;
            memory_pages_.insert(std::make_pair(std::make_pair(start, start + size - 1), page));
        }
        lastProcessesTickCount = tickCount;
    }
}

uint32_t GameMemory::mem_find_base_addr(uint32_t address, uint32_t* size)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // Search for the memory page address
    auto found = memory_pages_.find(std::make_pair(address, address));

    if(found == memory_pages_.end())
        return 0;

    // Return the allocation region size when requested
    if(size)
        *size = found->second.RegionSize;

    return found->first.first;
}

bool GameMemory::mem_get_page_info(uint32_t address, MEMORY_BASIC_INFORMATION* pageInfo)
{
    std::lock_guard<std::mutex> lock(mtx_);
    // Search for the memory page address
    auto found = memory_pages_.find(std::make_pair(address, address));

    if(found == memory_pages_.end())
        return false;

    // Return the data when possible
    if(pageInfo)
        *pageInfo = found->second;

    return true;
}

bool GameMemory::mem_is_code_page(uint32_t address)
{
    MEMORY_BASIC_INFORMATION pageInfo;
    if(!mem_get_page_info(address, &pageInfo))
        return false;

    return (pageInfo.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
}

std::string GameMemory::stack_get_call_stack(uint32_t csp,std::vector<uint32_t>& stack_call_back_list)
{
    std::string stack_call_back = "";
    if(csp % sizeof(uint32_t)) //alignment problem
        return stack_call_back;

    mem_update_map();

    if(!mem_isvalid_readptr(csp))
        return stack_call_back;
    uint32_t stack_size = 0;

    uint32_t stack_base = mem_find_base_addr(csp, &stack_size);
    if(!stack_base)
        return stack_call_back;
    // walk up the stack
    uint32_t i = csp;
    while(i != stack_base + stack_size)
    {
        uint32_t data = 0;
        mem_read(i, &data, sizeof(uint32_t), nullptr);
        if(mem_isvalid_readptr(data) && mem_is_code_page(data))
        {
            uint32_t size = 0,dest = 0;
            uint32_t base = mem_find_base_addr(data, &size);
            uint32_t read_start = data - 6;
            if(read_start < base)
                read_start = base;
            if(*LPBYTE(data - 5) == 0xE8)
            {
                dest = data + *PINT(data - 4);
                if(!mem_isvalid_readptr(dest) || !mem_is_code_page(dest))
                {
                    continue;
                }
            }
            if(*LPBYTE(data - 5) == 0xE8
                || *LPWORD(data - 6) == 0x15FF
                || (*LPBYTE(data - 2) == 0xFF && 0xD0 <= *LPBYTE(data - 1) && *LPBYTE(data - 1) <= 0xD7))
            {
                char hex_addr[20] = {0};
                sprintf_s(hex_addr, "%08X|", data);
                stack_call_back.append(hex_addr);
                stack_call_back_list.push_back(data);
            }
        }
        i += sizeof(uint32_t);
    }

    return std::move(stack_call_back);
}
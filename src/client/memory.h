#pragma once
#include <Windows.h>
#include <map>
#include <mutex>

typedef std::pair<uint32_t, uint32_t> Range;

struct range_compare
{
    bool operator()(const Range & a, const Range & b) const//a before b?
    {
        return a.second < b.first;
    }
};

class GameMemory
{
public:
    static GameMemory& instance();
    std::string stack_get_call_stack(uint32_t csp, std::vector<uint32_t>& stack_call_back_list);
private:
    GameMemory();
    ~GameMemory();
    bool memory_read_safe(HANDLE hprocess, LPVOID lp_base_address, LPVOID lpbuffer, SIZE_T nsize, SIZE_T * lp_number_of_bytes_read);
    bool mem_read(uint32_t base_address, void * buffer, uint32_t size, uint32_t * number_of_bytes_read);
    bool mem_isvalid_readptr(uint32_t address);
    void mem_update_map();
    uint32_t mem_find_base_addr(uint32_t address, uint32_t * size);
    bool mem_get_page_info(uint32_t address, MEMORY_BASIC_INFORMATION * pageInfo);
    bool mem_is_code_page(uint32_t address);

    HANDLE current_process_handle_;
    std::map<Range, MEMORY_BASIC_INFORMATION, range_compare> memory_pages_;
    std::mutex mtx_;
};
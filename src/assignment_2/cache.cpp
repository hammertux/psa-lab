#include "cache.h"
#include "util.h"


unsigned int Cache::get_lru_index(const set_t& set) const
{
    unsigned int lru_index = 0;
    cache_line_t highest; //dummy line for comparison
    highest.lru_age = 0;
    highest.line = {};
    highest.tag = 0;

    for(const auto& line : set) {
        if(line.lru_age == 0) {
            lru_index = &line - &set.at(0); //carve out index of line
            goto out;
        }
    }

    for(const auto& line : set) {
        if(highest < line) {
            highest = line;
            lru_index = &line - &set.at(0);
        }
    }

out:
    return lru_index;
}

void Cache::increment_age(set_t& set, cache_line_t& last_used_line) const
{

    for(auto& line : set) {
        if(line == last_used_line) {
            line.lru_age = 1;
        }
        else if(line.lru_age == 0) { //don't increment age of empty cache line
            continue;
        }
        else {
            ++line.lru_age;
        }
    }

}

inline int8_t Cache::is_hit(const cache_addr_t& addr) const
{
    auto cache_p = cache.get();
    int8_t hit = -1; //if no hit, return -1 to evict lru cache line
    std::ostringstream log;
    for(int8_t i = 0; i < SET_SIZE; ++i) {
        log << "Comparing line tag: " << cache_p->at(addr.set_addr).at(i).tag << " -- Addr tag: " << addr.tag;
        SC_REPORT_INFO(LOG_ID, log.str().c_str());
        if((cache_p->at(addr.set_addr).at(i).tag == addr.tag) && cache_p->at(addr.set_addr).at(i).valid == 1){
            SC_REPORT_INFO(LOG_ID, "Cache Hit");
            hit = i;
            break;
        }
        log.str("");
        log.clear();
    }

    return hit;
}

__always_inline void Cache::evict_entry(const cache_addr_t& addr, cache_line_t& evict_line) const
{
    for(auto& byte : evict_line.line) {
        byte = 0x42; 
    }
    sc_core::wait(MEM_LATENCY_CYCLES);
}

int Cache::write(const cache_addr_t& addr, const uint8_t data) 
{
    auto cache_p = cache.get();
    set_t *set = &cache_p->at(addr.set_addr);
    int retv = 1;
    int8_t line_index = is_hit(addr);
    unsigned int lru_index = get_lru_index(*set);

    if(line_index != -1) {
        //Write hit, write through via bus an Invalidation signal so other caches invalidate line
        SC_REPORT_INFO(LOG_ID, "Cache write hit");
        retv = 0;
        PUT_DATA((*set), line_index, addr, data);
        increment_age(*set, set->at(line_index));
        goto out_hit;
    }
    else {
        //Write miss, read from bus, evict lru line and replace it.
        SC_REPORT_INFO(LOG_ID, "Cache write miss");
        retv = 1;
        memory->write(addr, RANDOM_DATA);
        evict_entry(addr, set->at(lru_index));
        PUT_DATA((*set), lru_index, addr, data);
        UPDATE_TAG((*set), lru_index, addr);
        set->at(lru_index).valid = 1;
        increment_age(*set, set->at(lru_index));
        goto out_miss;
    }

out_hit:
    stats_writehit(CPUID);
    sc_core::wait(CACHE_LATENCY_CYCLES);
    return retv;
out_miss:
    stats_writemiss(CPUID);
    sc_core::wait(MEM_LATENCY_CYCLES);
    return retv;
}


int Cache::read(const cache_addr_t& addr) 
{
    auto cache_p = cache.get();
    set_t *set = &cache_p->at(addr.set_addr);
    int retv = 1;
    int8_t line_index = 0;
    unsigned int lru_index = get_lru_index(*set);
    uint8_t data;

    if((line_index = is_hit(addr)) != -1) {
        //Read Hit no bus operation required
        SC_REPORT_INFO(LOG_ID, "Cache read hit");
        retv = 0;
        data = GET_DATA((*set), line_index, addr);
        //port_data.write(data);
        increment_age(*set, set->at(line_index));
        goto out_hit;
    }
    else {
        //Read Miss, read from bus (main memory), evict lru, replace with line from bus
        SC_REPORT_INFO(LOG_ID, "Cache read miss");
        retv = 1;
        evict_entry(addr, set->at(lru_index));
        memory->read(addr);
        data = GET_DATA((*set), lru_index, addr);
        UPDATE_TAG((*set), lru_index, addr);
        set->at(lru_index).valid = 1;
        //port_data.write(data);
        increment_age(*set, set->at(lru_index));
        goto out_miss;
    }


out_hit:
    stats_readhit(CPUID);
    sc_core::wait(CACHE_LATENCY_CYCLES);
    return retv;
out_miss:
    stats_readmiss(CPUID);
    sc_core::wait(MEM_LATENCY_CYCLES);
    return retv;

}

void Cache::snoop()
{
    auto cache_p = cache.get();
    cache_addr_t addr;
    set_t *set = nullptr;
    addr_id_pair_t bus_pair;

    while(1) {
        wait(port_bus_in->value_changed_event());
        bus_pair = port_bus_in->read();
        addr.memory_addr = bus_pair.first;
        set = &cache_p->at(addr.set_addr);

        for(auto& line : *set) {
            if(line.tag == addr.tag && this->cpuid != bus_pair.second) {
                INVALIDATE_LINE(line);
            }
        }
    }

}

// void Cache::execute()
// {   
//     cache_addr_t addr;
//     Function func;
//     uint8_t data;
//     std::ostringstream log;
    
//     while(1) {
//         //wait(port_func.value_changed_event());
//         SC_REPORT_INFO(LOG_ID, "\nReceived Function Signal!\n");
//         //func = port_func.read();
//         //addr.memory_addr = port_addr.read();
//         data = 0;
//         log << "Tag: " << addr.tag << " -- Set: " << addr.set_addr << " -- Offset: " << addr.byte_offset;
//         SC_REPORT_INFO(LOG_ID, log.str().c_str());
//         if(func == FUNC_WRITE) {
//             SC_REPORT_INFO(LOG_ID, "Executing Cache Write");
//             //data = static_cast<uint8_t>(port_data.read().to_int());
//             this->write(addr, data);
//             //port_done.write(RET_WRITE);
//         }
//         else if(func == FUNC_READ) {
//             SC_REPORT_INFO(LOG_ID, "Executing Cache Read");
//             this->read(addr);
//             //port_done.write(RET_READ);
//         }
//         else {
//             throw std::runtime_error("Unknown Function");
//         }
//         log.str("");
//         log.clear();
//     }

// }
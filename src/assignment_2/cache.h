#ifndef __CACHE_H__
#define __CACHE_H__

#include <systemc>
#include <memory>
#include <array>
#include <stdint.h>
#include <sstream>
#include "cache_if.h"




#define SET_SIZE 8
#define CACHE_LINE_SIZE 32
#define CACHE_SIZE (1 << 15) //32KB
#define TOTAL_SETS ((CACHE_SIZE / CACHE_LINE_SIZE) / SET_SIZE)
#define TOTAL_CACHE_LINES (1 << 10) //1024
#define MEM_LATENCY_CYCLES 100
#define CACHE_LATENCY_CYCLES 1
#define CPUID 0
#define LOG_ID 42

#define UPDATE_TAG(s, l, a)   \
    s.at(l).tag = a.tag        

#define PUT_DATA(s, l, a, d)  \
    s.at(l).line.at(a.byte_offset) = d

#define GET_DATA(s, l, a)     \
    s.at(l).line.at(a.byte_offset)

using namespace sc_core;



typedef union __addr{ //to avoid bitmasking and uses only the 4 bytes of the address, no extra vars needed
    struct {
        uint32_t byte_offset:5;
        uint32_t set_addr:7;
        uint32_t tag:20;
    }__attribute((packed, aligned(4))); //make sure it's no bigger than 4B and aligns to 4B boundary
    uint32_t memory_addr;
} cache_addr_t;

typedef struct __line {
    std::array<uint8_t, CACHE_LINE_SIZE> line;
    struct{
        int32_t tag; //need only 20bits, extra 12 will be useful in the next labs for state.
        uint32_t lru_age; // lru counter. Assuming no cache line will stay for >2^32 accesses as unused.
    };
    inline bool operator<(const struct __line& line) const {
        return this->lru_age < line.lru_age;
    }
    inline bool operator==(const struct __line& line) const {
        return ((this->tag == line.tag) && (this->line == line.line));
    }
} cache_line_t;

typedef std::array<cache_line_t, SET_SIZE> set_t;

class Cache : public Cache_if, sc_module {
    private:
        
        std::unique_ptr<std::array<set_t, TOTAL_SETS>> cache;

        void execute();
        unsigned int get_lru_index(const set_t&) const;
        
        inline int8_t is_hit(const cache_addr_t&) const;
        void increment_age(set_t&, cache_line_t&) const;
        __always_inline void evict_entry(const cache_addr_t&, cache_line_t&) const;

    public:
        enum CacheRetCode {
            CACHE_HIT,
            CACHE_MISS
        };

        enum Function {
            FUNC_READ,
            FUNC_WRITE
        };

        enum FuncRetCode {
            RET_READ,
            RET_WRITE
        };

        sc_in<bool> port_clk;
        sc_in<Function> port_func;
        sc_in<uint32_t> port_addr;
        sc_out<FuncRetCode> port_done;
        sc_inout_rv<8> port_data;

        //memory ports
        sc_in<Memory::RetCode>     port_mem_done;
        sc_out<Memory::Function>   port_mem_func;
        sc_out<uint32_t>                port_mem_addr;
        sc_inout_rv<8>            port_mem_data;

        SC_CTOR(Cache) : cache(new std::array<set_t, TOTAL_SETS>)
        {
            if(!cache) {
                throw std::runtime_error("Couldn't initialise cache.");
            }
            for(auto& set : *(cache.get())) {
                for(auto& line : set) {
                    line.line = {};
                    line.tag = -1;
                    line.lru_age = 0;
                }
            }
            SC_THREAD(execute);
            sensitive << port_clk.pos();
            dont_initialize();
            sc_report::register_id(LOG_ID, "[SC_LOG] ");
            sc_report_handler::set_actions (SC_ID_VECTOR_CONTAINS_LOGIC_VALUE_,
                                SC_DO_NOTHING);
            sc_report_handler::set_actions( "/IEEE_Std_1666/deprecated",
                                SC_DO_NOTHING );
            
        }

        ~Cache(){}

        CacheRetCode write(const cache_addr_t&, set_t&, uint8_t) const;
        CacheRetCode read(const cache_addr_t&, set_t&);


};

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
        if((cache_p->at(addr.set_addr).at(i).tag == addr.tag)){
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
}

Cache::CacheRetCode Cache::write(const cache_addr_t& addr, set_t& set, uint8_t data) const
{
    CacheRetCode retv = CACHE_MISS;
    int8_t line_index = is_hit(addr);
    unsigned int lru_index = get_lru_index(set);

    if(line_index != -1) {
        SC_REPORT_INFO(LOG_ID, "Cache write hit");
        retv = CACHE_HIT;
        PUT_DATA(set, line_index, addr, data);
        increment_age(set, set.at(line_index));
        goto out_hit;
    }
    else {
        SC_REPORT_INFO(LOG_ID, "Cache write miss");
        retv = CACHE_MISS;
        evict_entry(addr, set.at(lru_index));
        PUT_DATA(set, lru_index, addr, data);
        UPDATE_TAG(set, lru_index, addr);
        increment_age(set, set.at(lru_index));
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


Cache::CacheRetCode Cache::read(const cache_addr_t& addr, set_t& set) 
{
    CacheRetCode retv = CACHE_MISS;
    int8_t line_index = 0;
    unsigned int lru_index = get_lru_index(set);
    uint8_t data;

    if((line_index = is_hit(addr)) != -1) {
        SC_REPORT_INFO(LOG_ID, "Cache read hit");
        retv = CACHE_HIT;
        data = GET_DATA(set, line_index, addr);
        port_data.write(data);
        increment_age(set, set.at(line_index));
        goto out_hit;
    }
    else {
        SC_REPORT_INFO(LOG_ID, "Cache read miss");
        retv = CACHE_MISS;
        evict_entry(addr, set.at(lru_index));
        data = GET_DATA(set, lru_index, addr);
        UPDATE_TAG(set, lru_index, addr);
        port_data.write(data);
        increment_age(set, set.at(lru_index));
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

void Cache::execute()
{   
    auto cache_p = cache.get();
    cache_addr_t addr;
    Function func;
    uint8_t data;
    std::ostringstream log;
    
    while(1) {
        wait(port_func.value_changed_event());
        SC_REPORT_INFO(LOG_ID, "\nReceived Function Signal!\n");
        func = port_func.read();
        addr.memory_addr = port_addr.read();
        data = 0;
        log << "Tag: " << addr.tag << " -- Set: " << addr.set_addr << " -- Offset: " << addr.byte_offset;
        SC_REPORT_INFO(LOG_ID, log.str().c_str());
        if(func == FUNC_WRITE) {
            SC_REPORT_INFO(LOG_ID, "Executing Cache Write");
            data = static_cast<uint8_t>(port_data.read().to_int());
            this->write(addr, cache_p->at(addr.set_addr), data);
            port_done.write(RET_WRITE);
        }
        else if(func == FUNC_READ) {
            SC_REPORT_INFO(LOG_ID, "Executing Cache Read");
            this->read(addr, cache_p->at(addr.set_addr));
            port_done.write(RET_READ);
        }
        else {
            throw std::runtime_error("Unknown Function");
        }
        log.str("");
        log.clear();
    }

}

#endif /* __CACHE_H__ */
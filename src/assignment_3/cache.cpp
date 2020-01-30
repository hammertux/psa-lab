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
        if((cache_p->at(addr.set_addr).at(i).tag == addr.tag) && cache_p->at(addr.set_addr).at(i).valid == 1){
            hit = i;
            break;
        }
        log.str("");
        log.clear();
    }

    return hit;
}

__always_inline void Cache::evict_entry(const cache_addr_t& addr, cache_line_t& evict_line)
{
    if(evict_line.modified || evict_line.owned) {
        bus->write(this->cpuid, addr.memory_addr, RANDOM_DATA);//WB to memory
    }
}

int Cache::write(const cache_addr_t& addr, const uint8_t data) 
{
    std::cout << "CACHE ID = " << this->cpuid << "Addr = " << addr.memory_addr << std::endl;
    auto cache_p = cache.get();
    set_t *set = &cache_p->at(addr.set_addr);
    int retv = 1;
    int8_t line_index = is_hit(addr);
    unsigned int lru_index = get_lru_index(*set);
    bus_sig_t sig;

    if(line_index != -1) {
        //Write hit, write through via bus an Invalidation signal so other caches invalidate line
        //change into modified state
        retv = 0;
        PUT_DATA((*set), line_index, addr, data);
        bus->write(this->cpuid, addr.memory_addr, RANDOM_DATA);
        wait(port_bus_in->value_changed_event());
        sig = port_bus_in->read();
        while(sig.req_status != REQ_CACHE_DONE && sig.id == this->cpuid) {wait();}
        increment_age(*set, set->at(line_index));
        if(set->at(line_index).exclusive || set->at(line_index).shared || set->at(line_index).owned) {
            CLEAR_MOESI_BITS(set->at(line_index));
            set->at(line_index).valid = 1;
            set->at(line_index).modified = 1;
        }
        goto out_hit;
    }
    else {
        //Write miss, read from bus, evict lru line and replace it. invalidate other lines/write through to memory
        retv = 1;
        evict_entry(addr, set->at(lru_index));
        bus->read(this->cpuid, addr.memory_addr);
        wait(port_bus_in->value_changed_event());
        sig = port_bus_in->read();
        while(sig.req_status != REQ_CACHE_DONE && sig.id == this->cpuid) {wait();}
        PUT_DATA((*set), lru_index, addr, data);
        UPDATE_TAG((*set), lru_index, addr);
        set->at(lru_index).valid = 1;
        set->at(lru_index).modified = 1;
        increment_age(*set, set->at(lru_index));
        goto out_miss;
    }

out_hit:
    stats_writehit(cpuid);
    sc_core::wait(CACHE_LATENCY_CYCLES);
    return retv;
out_miss:
    stats_writemiss(cpuid);
    //std:: cout << "Write miss" << std::endl;
    return retv;
}


int Cache::read(const cache_addr_t& addr) 
{
    std::cout << "CACHE ID = " << this->cpuid << "Addr = " << addr.memory_addr << std::endl;
    auto cache_p = cache.get();
    set_t *set = &cache_p->at(addr.set_addr);
    int retv = 1;
    int8_t line_index = 0;
    unsigned int lru_index = get_lru_index(*set);
    uint8_t data;
    bus_sig_t sig;

    if((line_index = is_hit(addr)) != -1) {
        //Read Hit no bus operation required
        retv = 0;
        data = GET_DATA((*set), line_index, addr);
        increment_age(*set, set->at(line_index));
        goto out_hit;
    }
    else {
        //Read Miss, read from bus (main memory), evict lru, replace with line from bus
        retv = 1;
        CLEAR_MOESI_BITS(set->at(lru_index));
        evict_entry(addr, set->at(lru_index));
        bus->read(this->cpuid, addr.memory_addr);
        wait(port_bus_in->value_changed_event());
        sig = port_bus_in->read();
        while(sig.req_status != REQ_CACHE_DONE && sig.id == this->cpuid) {wait();}
        if(sig.is_c2c) {
            set->at(lru_index).shared = 1;
        }
        else {
            set->at(lru_index).exclusive = 1;
        }
        data = GET_DATA((*set), lru_index, addr);
        UPDATE_TAG((*set), lru_index, addr);
        set->at(lru_index).valid = 1;
        increment_age(*set, set->at(lru_index));
        goto out_miss;
    }


out_hit:
    stats_readhit(cpuid);
    sc_core::wait(CACHE_LATENCY_CYCLES);
    //std::cout << "read hit" << std::endl;
    return retv;
out_miss:
    stats_readmiss(cpuid);
    return retv;

}

void Cache::snoop()
{
    auto cache_p = cache.get();
    cache_addr_t addr;
    set_t *set = nullptr;
    bus_sig_t bus_sig;

    while(1) {
        wait(port_bus_in->value_changed_event() | port_c2c_in->value_changed_event());
        bus_sig = port_bus_in->read();
        addr.memory_addr = bus_sig.addr;
        set = &cache_p->at(addr.set_addr);
        if(bus_sig.req_status == REQ_CACHE_DONE) {
            for(auto& line : *set) {
                if(line.tag == addr.tag && bus_sig.b == BUS_WRITE){
                    if(this->cpuid != bus_sig.id) {
                        if(line.owned || line.exclusive || line.modified || line.shared) {
                            INVALIDATE_LINE(line);
                        }
                    }
                    else {
                        if(line.valid == 0)
                        line.valid = 1;
                        line.modified = 1;
                        line.line.at(addr.byte_offset) = RANDOM_DATA;
                    }
                }
                else if(line.tag == addr.tag && bus_sig.b == BUS_READ) {
                    
                    if(this->cpuid != bus_sig.id) {
                        std::cout << "snoop read happened " << std::endl;
                        if(line.exclusive) {
                            CLEAR_MOESI_BITS(line);
                            line.valid = 1;
                            line.shared = 1;
                        }
                        else if(line.modified) {
                            std::cout << "M -> O" << std::endl;
                            CLEAR_MOESI_BITS(line);
                            line.valid = 1;
                            line.owned = 1;
                        }
                    }
                }
            }
        }
        else {
            bus_sig = port_c2c_in->read();
            addr.memory_addr = bus_sig.addr;
            set = &cache_p->at(addr.set_addr);
            std::cout << "snooped this sig: " << bus_sig << std::endl;
        
            for(auto& line : *set) {
                if(line.tag == addr.tag && (line.owned || line.modified || line.shared)){
                    if(this->cpuid != bus_sig.id) {
                        std::cout << "I can satisfy that :)" << std::endl;
                        bus->cache_to_cache(bus_sig.id, bus_sig.addr);
                    }
                }
            }
            //check if I have a copy of the requested data, if i do, serve it via the bus.
            //should I serve only if i am the owner? If not how can I handle mutliple writers to signal?
            //latency cycles for cache-to-cache transfers?
        }
    }

}
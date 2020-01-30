#ifndef __CACHE_H__
#define __CACHE_H__

#include <systemc>
#include <memory>
#include <array>
#include <stdint.h>
#include <sstream>
#include "cache_if.h"
#include "util.h"
#include "bus_slave_if.h"
#include "bus_if.h"


using namespace sc_core;



#define UPDATE_TAG(s, l, a)   \
    s.at(l).tag = a.tag        

#define PUT_DATA(s, l, a, d)  \
    s.at(l).line.at(a.byte_offset) = d

#define GET_DATA(s, l, a)     \
    s.at(l).line.at(a.byte_offset)

#define INVALIDATE_LINE(l)    \
    l.valid = 0; \
    l.modified = 0; \
    l.shared = 0; \
    l.owned = 0; \
    l.exclusive = 0

#define CLEAR_MOESI_BITS(l)   \
    l.valid = 0; \
    l.modified = 0; \
    l.shared = 0; \
    l.owned = 0; \
    l.exclusive = 0


class Cache : public Cache_if, public sc_module {
    private:
        uint16_t cpuid;
        std::unique_ptr<std::array<set_t, TOTAL_SETS>> cache;
        

        void snoop();
        unsigned int get_lru_index(const set_t&) const;
        
        inline int8_t is_hit(const cache_addr_t&) const;
        void increment_age(set_t&, cache_line_t&) const;
        __always_inline void evict_entry(const cache_addr_t&, cache_line_t&);

    public:

        sc_port<Bus_if> bus;
        sc_in_clk port_clk;
        sc_port<sc_signal_in_if<bus_sig_t>> port_bus_in;
        sc_port<sc_signal_in_if<bus_sig_t>> port_c2c_in;

        Cache(sc_module_name __name, uint16_t __cpuid) : sc_module(__name),
                                                         cpuid(__cpuid),
                                                         cache(new std::array<set_t, TOTAL_SETS>)
                                                         
        {
            SC_THREAD(snoop);
            sensitive << port_clk.pos();
            
            if(!cache) {
                throw std::runtime_error("Couldn't initialise cache.");
            }
            for(auto& set : *(cache.get())) {
                for(auto& line : set) {
                    line.line = {};
                    line.tag = 0;
                    line.valid = 0;
                    line.lru_age = 0;
                    line.modified = 0;
                    line.owned = 0;
                    line.shared = 0;
                }
            }
            
            
            
            sc_report::register_id(LOG_ID, "[SC_LOG] ");
            sc_report_handler::set_actions (SC_ID_VECTOR_CONTAINS_LOGIC_VALUE_,
                                SC_DO_NOTHING);
            sc_report_handler::set_actions( "/IEEE_Std_1666/deprecated",
                                SC_DO_NOTHING );
            
        }
        SC_HAS_PROCESS(Cache);

        ~Cache(){}

        int write(const cache_addr_t&, const uint8_t) override;
        int read(const cache_addr_t&) override;


};



#endif /* __CACHE_H__ */
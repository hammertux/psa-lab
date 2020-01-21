#ifndef __CACHE_H__
#define __CACHE_H__

#include <systemc>
#include <memory>
#include <array>
#include <stdint.h>
#include <sstream>
#include "cache_if.h"
#include "util.h"






#define UPDATE_TAG(s, l, a)   \
    s.at(l).tag = a.tag        

#define PUT_DATA(s, l, a, d)  \
    s.at(l).line.at(a.byte_offset) = d

#define GET_DATA(s, l, a)     \
    s.at(l).line.at(a.byte_offset)

using namespace sc_core;



class Cache : public Cache_if, public sc_module {
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

        SC_CTOR(Cache) : cache(new std::array<set_t, TOTAL_SETS>)
        {
            if(!cache) {
                throw std::runtime_error("Couldn't initialise cache.");
            }
            for(auto& set : *(cache.get())) {
                for(auto& line : set) {
                    line.line = {};
                    line.tag = 0;
                    line.valid = 0;
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

        int write(const cache_addr_t&, uint8_t&) override;
        int read(const cache_addr_t&) override;


};



#endif /* __CACHE_H__ */
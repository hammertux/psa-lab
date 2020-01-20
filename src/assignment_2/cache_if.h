#ifndef __CACHE_IF_H__
#define __CACHE_IF_H__

#include <systemc>

using namespace sc_core;

typedef union __addr{ //to avoid bitmasking and uses only the 4 bytes of the address, no extra vars needed
    struct {
        uint32_t byte_offset:5;
        uint32_t set_addr:7;
        uint32_t tag:20;
    }__attribute((packed, aligned(4))); //make sure it's no bigger than 4B and aligns to 4B boundary
    uint32_t memory_addr;
} cache_addr_t;

class Cache_if : public virtual sc_interface {
    public:
        virtual int read(const cache_addr_t&) = 0;
        virtual int write(const cache_addr_t&) = 0;

        virtual ~Cache_if();
};



#endif /* __CACHE_IF_H__ */
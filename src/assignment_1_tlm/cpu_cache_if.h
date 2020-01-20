#include <systemc.h>

#ifndef CPU_CACHE_IF_H
#define CPU_CACHE_IF_H

class cpu_cache_if : public virtual sc_interface {
    public:
    virtual int cpu_read(uint32_t addr) = 0;
    virtual int cpu_write(uint32_t addr) = 0;
};

#endif

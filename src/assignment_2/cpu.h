#ifndef __CPU_H__
#define __CPU_H__

#include <systemc>
#include "cache.h"
#include "psa.h"



class CPU : public sc_module {

    public:
        sc_in_clk clock;
        sc_port<Cache_if> cache;

        CPU(sc_module_name __name, uint16_t __cpuid) : sc_module(__name), 
                                                       cpuid(__cpuid) {
            SC_THREAD(execute);
            sensitive << clock.pos();
            dont_initialize();
        }

        SC_HAS_PROCESS(CPU);

    private:
        uint16_t cpuid;

        void execute();
};


#endif /* __CPU_H__ */
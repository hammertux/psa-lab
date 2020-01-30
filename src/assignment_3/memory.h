#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <systemc>
#include "bus_slave_if.h"
#include "util.h"


#define MEM_SIZE 512

using namespace sc_core;

class Memory : public virtual Bus_slave_if, public sc_module
{

public:
    long mem_write_access_rates;
    long mem_read_access_rates;

    SC_CTOR(Memory)
    {
        // SC_THREAD(execute);
        // sensitive << Port_CLK.pos(); //sensitive to rising edge of clock
        // dont_initialize();

        // m_data = new int[MEM_SIZE];
        mem_read_access_rates = 0;
        mem_write_access_rates = 0;
    }

    ~Memory()
    {
        // delete[] m_data;
    }

private:
    int* m_data;

    MEM_REQ_STATUS read(const uint32_t) override {wait(MEM_LATENCY_CYCLES); mem_read_access_rates++; return REQ_MEM_DONE;}
    MEM_REQ_STATUS write(const uint32_t, uint8_t data) override {wait(MEM_LATENCY_CYCLES); mem_write_access_rates++; return REQ_MEM_DONE;}
};


#endif /* __MEMORY_H__ */
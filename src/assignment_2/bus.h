#ifndef __BUS_H__
#define __BUS_H__

#include "bus_if.h"

using namespace sc_core;

class Bus : public Bus_if, public sc_module {
    private:
        sc_in<bool> port_clk;
        sc_signal_rv<32> port_bus_addr;
    public:
        SC_CTOR(Bus) {
            sensitive << port_clk.neg();
        }
};



#endif /* __BUS_H__ */
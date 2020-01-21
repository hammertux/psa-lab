#ifndef __BUS_H__
#define __BUS_H__

#include "bus_if.h"
#include "util.h"

using namespace sc_core;

class Bus : public Bus_if, public sc_module {
    private:

        void execute();
    public:
        sc_in<bool> port_clk;
        sc_port<sc_signal_inout_if<addr_id_pair_t>> port_bus_inout;

        SC_CTOR(Bus) {
            sensitive << port_clk.neg();
            SC_THREAD(execute);
        }

        int read(uint32_t) override;
        int write(uint32_t, uint8_t) override;
};





#endif /* __BUS_H__ */
#ifndef __BUS_H__
#define __BUS_H__

#include "bus_if.h"
#include "util.h"
#include "bus_slave_if.h"
#include "bus_arbiter.h"
#include <algorithm>

using namespace sc_core;

class Bus : public Bus_if, public sc_module {
    private:
        uint64_t total_requests_issued;
        std::vector<std::shared_ptr<bus_sig_t>> requests;
        std::shared_ptr<bus_sig_t> current_req;
        
        void execute();
    public:
        sc_in<bool> port_clk;
        sc_port<Bus_arbiter_if> arbiter;
        sc_port<Bus_slave_if> memory;
        sc_port<sc_signal_inout_if<bus_sig_t>, SC_MANY_WRITERS> port_bus_inout;

        SC_CTOR(Bus) : current_req(nullptr){
            
            SC_CTHREAD(execute, port_clk.neg());
            //dont_initialize();
            sensitive << port_clk.neg();
            
        }
        void read(uint16_t, uint32_t) override;
        void write(uint16_t, uint32_t, uint8_t) override;
};





#endif /* __BUS_H__ */
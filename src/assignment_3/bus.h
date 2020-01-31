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
        size_t last_queue_size;
        bus_sig_t old_tail;
        
        void execute();
    public:
        sc_in<bool> port_clk;
        sc_port<Bus_arbiter_if> arbiter;
        sc_port<Bus_slave_if> memory;
        sc_port<sc_signal_inout_if<bus_sig_t>> port_bus_inout;
        sc_port<sc_signal_inout_if<bus_sig_t>> port_c2c_inout;
        sc_port<sc_signal_inout_if<bus_sig_t>> port_advertise_inout;

        SC_CTOR(Bus) : current_req(nullptr){
            
            SC_CTHREAD(execute, port_clk.neg());
            //dont_initialize();
            sensitive << port_clk.neg();
            last_queue_size = 0;
            old_tail = bus_sig_t();
            
        }
        void read(uint16_t, uint32_t) override;
        void write(uint16_t, uint32_t, uint8_t) override;
        void cache_to_cache(uint16_t, uint32_t) override;
};





#endif /* __BUS_H__ */
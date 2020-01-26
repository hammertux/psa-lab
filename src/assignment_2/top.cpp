#include <systemc>
#include "psa.h"
#include "memory.h"
#include "cache.h"
#include "cpu.h"
#include "bus.h"
#include "bus_arbiter.h"
#include <vector>

using namespace sc_core;

int sc_main(int argc, char* argv[])
{
    try
    {
        // Get the tracefile argument and create Tracefile object
        // This function sets tracefile_ptr and num_cpus
        init_tracefile(&argc, &argv);

        // Initialize statistics counters
        stats_init();

        std::vector<std::shared_ptr<CPU>> cpus(num_cpus);
        std::vector<std::shared_ptr<Cache>> caches(num_cpus);
        std::shared_ptr<Memory> mem;
        std::shared_ptr<Bus> bus;
        std::shared_ptr<Arbiter> arbiter;
        sc_clock clk;
        sc_signal<bus_sig_t> bus_sig;

        mem = std::make_shared<Memory>("main_memory");
        bus = std::make_shared<Bus>("bus");
        arbiter = std::make_shared<Arbiter>("arbiter");
        if(!mem || !bus || !arbiter) {
            throw std::runtime_error("Uninit components");
        }
        bus.get()->port_clk(clk);
        bus.get()->port_bus_inout(bus_sig);
        bus.get()->arbiter(*(arbiter).get());
        bus.get()->memory(*(mem).get());
        
        for(uint16_t i = 0; i < num_cpus; i++) {
            
            cpus.at(i) = std::make_shared<CPU>((("cpu_" + std::to_string(i)).c_str()), i);
            caches.at(i) = std::make_shared<Cache>((("cache_" + std::to_string(i)).c_str()), i);
            cpus.at(i).get()->cache(*(caches.at(i).get()));
            cpus.at(i).get()->clock(clk);
            caches.at(i).get()->bus(*(bus.get()));
            caches.at(i).get()->port_bus_in(bus_sig);
            caches.at(i).get()->port_clk(clk);
            //std::cout << "I VAL = " << i << std::endl;
        }
        

        std::cout << "Running (press CTRL+C to interrupt)... " << std::endl;


        // Start Simulation
        sc_start();

        // Print statistics after simulation finished
        stats_print();
        std::cout << "Total Memory Accesses = " << mem->mem_access_rates << std::endl;
    }

    catch (std::exception& e)
    {
        SC_REPORT_ERROR(LOG_ID, e.what());
    }

    return 0;
}
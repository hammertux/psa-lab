/*
// File: top.cpp
//
*/

#include <systemc.h>
#include <iostream>

#include "psa.h"
#include "Cache.h"
#include "CPU.h"

using namespace std;

int sc_main(int argc, char* argv[]) {
    try {
        // Get the tracefile argument and create Tracefile object
        // This function sets tracefile_ptr and num_cpus
        init_tracefile(&argc, &argv);

        // init_tracefile changed argc and argv so we cannot use
        // getopt anymore. "-q" must be specified after the tracefile.
        if (argc == 2 && !strcmp(argv[0], "-q")) {
            sc_report_handler::set_verbosity_level(SC_LOW);
        }

        sc_set_time_resolution(1, SC_PS);

        // Initialize statistics counters
        stats_init();

        // Create instances with id 0
        CPU* cpu = new CPU("cpu", 0);
        Cache* cache =  new Cache("cache", 0);
        Memory* memory = new Memory("memory");

        // The clock that will drive the CPU
        sc_clock clk;

        // Connect instances
        cpu->cache(*cache);
        cache->memory(*memory);

        cpu->clock(clk);

        // Start Simulation
        sc_start();

        // Print statistics after simulation finished
        stats_print();
        cout << sc_time_stamp() << endl;

        // Cleanup components
        delete cpu;
        delete cache;
        delete memory;
    } catch (exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}

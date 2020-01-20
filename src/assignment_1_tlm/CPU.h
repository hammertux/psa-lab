#ifndef CPU_H
#define CPU_H

#include <systemc.h>
#include <iostream>

#include "cpu_cache_if.h"
#include "helpers.h"
#include "psa.h"

class CPU : public sc_module {
public:
    sc_in_clk clock;
    sc_port<cpu_cache_if> cache;

    CPU(sc_module_name name_, int id_) : sc_module(name_), id(id_) {
        SC_THREAD(execute);
        sensitive << clock.pos();
        log(name(), "constructed with id", id);
        dont_initialize(); // don't call execute to initialise it.
    }

    SC_HAS_PROCESS(CPU); // Needed because we didn't use SC_TOR

private:
    int id;

    void execute() {
        TraceFile::Entry    tr_data;
        // Loop until end of tracefile
        while (!tracefile_ptr->eof()) {
            // Get the next action for the processor in the trace
            if (!tracefile_ptr->next(id, tr_data)) {
                cerr << "Error reading trace for CPU" << endl;
                break;
            }

            switch (tr_data.type) {
                case TraceFile::ENTRY_TYPE_READ:
                    log(name(), "reading from address", tr_data.addr);
                    cache->cpu_read(tr_data.addr);
                    log(name(), "read done");
                    break;
                case TraceFile::ENTRY_TYPE_WRITE:
                    log(name(), "writing to address", tr_data.addr);
                    cache->cpu_write(tr_data.addr);
                    log(name(), "write done");
                    break;
                case TraceFile::ENTRY_TYPE_NOP:
                    log(name(), "nop");
                    break;
                default:
                    cerr << "Error, got invalid data from Trace" << endl;
                    exit(0);
            }
            wait();

        }
        // Finished the Tracefile, now stop the simulation
        sc_stop();
    }
};

#endif

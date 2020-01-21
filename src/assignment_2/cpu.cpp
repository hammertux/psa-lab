#include <systemc>
#include "util.h"
#include "cpu.h"


void CPU::execute()
    {
        TraceFile::Entry    tr_data;
        cache_addr_t addr;

        // Loop until end of tracefile
        while(!tracefile_ptr->eof())
        {
            // Get the next action for the processor in the trace
            if(!tracefile_ptr->next(0, tr_data))
            {
                std::runtime_error("Error reading trace for CPU");
            }

            addr.memory_addr = tr_data.addr;

            switch(tr_data.type)
            {
                case TraceFile::ENTRY_TYPE_READ:
                    cache->read(addr);
                    break;

                case TraceFile::ENTRY_TYPE_WRITE:
                    cache->write(addr, RANDOM_DATA);
                    break;

                case TraceFile::ENTRY_TYPE_NOP:
                    break;

                default:
                    throw std::runtime_error("Error, got invalid data from Trace");
            }
            // Advance one cycle in simulated time
            wait();
        }

        // Finished the Tracefile, now stop the simulation
        if(cpuid == 0){
            sc_stop();
        }
    }
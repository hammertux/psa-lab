#ifndef __CPU_H__
#define __CPU_H__

#include <systemc>
#include "cache.h"
#include "psa.h"

using namespace sc_core;

SC_MODULE(CPU)
{


public:
    sc_in<bool>                Port_CLK;
    sc_in<Cache::FuncRetCode>   Port_MemDone;
    sc_out<Cache::Function> Port_MemFunc;
    sc_out<uint32_t>                Port_MemAddr;
    sc_inout_rv<8>            Port_MemData;

    SC_CTOR(CPU)
    {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos();
        dont_initialize();
    }

private:
    void execute()
    {
        TraceFile::Entry    tr_data;
        Cache::Function  f;

        // Loop until end of tracefile
        while(!tracefile_ptr->eof())
        {
            // Get the next action for the processor in the trace
            if(!tracefile_ptr->next(0, tr_data))
            {
                std::cerr << "Error reading trace for CPU" << std::endl;
                break;
            }

            switch(tr_data.type)
            {
                case TraceFile::ENTRY_TYPE_READ:
                    f = Cache::FUNC_READ;
                    break;

                case TraceFile::ENTRY_TYPE_WRITE:
                    f = Cache::FUNC_WRITE;
                    break;

                case TraceFile::ENTRY_TYPE_NOP:
                    break;

                default:
                    std::cerr << "Error, got invalid data from Trace" << std::endl;
                    exit(1);
            }

            if(tr_data.type != TraceFile::ENTRY_TYPE_NOP)
            {
                Port_MemAddr.write(tr_data.addr);
                Port_MemFunc.write(f);

                if (f == Cache::FUNC_WRITE)
                {
                    std::cout << sc_time_stamp() << ": CPU sends write" << std::endl;

                    uint32_t data = rand();
                    Port_MemData.write(data);
                    wait();
                    Port_MemData.write("ZZZZZZZZ");
                }
                else
                {
                    std::cout << sc_time_stamp() << ": CPU sends read" << std::endl;
                }

                wait(Port_MemDone.value_changed_event());

                if (f == Cache::FUNC_READ)
                {
                    std::cout << sc_time_stamp() << ": CPU reads: " << Port_MemData.read() << std::endl;
                }
            }
            else
            {
                std::cout << sc_time_stamp() << ": CPU executes NOP" << std::endl;
            }
            // Advance one cycle in simulated time
            wait();
        }

        // Finished the Tracefile, now stop the simulation
        sc_stop();
    }
};


#endif /* __CPU_H__ */
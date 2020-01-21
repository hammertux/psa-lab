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

    SC_CTOR(Memory)
    {
        // SC_THREAD(execute);
        // sensitive << Port_CLK.pos(); //sensitive to rising edge of clock
        // dont_initialize();

        // m_data = new int[MEM_SIZE];
    }

    ~Memory()
    {
        // delete[] m_data;
    }

private:
    int* m_data;

    int read(const cache_addr_t&) override {wait(MEM_LATENCY_CYCLES); return 0;}
    int write(const cache_addr_t&, uint8_t data) override {wait(MEM_LATENCY_CYCLES); return 0;}

    // void execute()
    // {
    //     while (true)
    //     {
    //         wait(Port_Func.value_changed_event());

    //         Function f = Port_Func.read();
    //         int addr   = Port_Addr.read();
    //         int data   = 0;
    //         if (f == FUNC_WRITE)
    //         {
    //             std::cout << sc_time_stamp() << ": MEM received write" << std::endl;
    //             data = Port_Data.read().to_int();
    //         }
    //         else
    //         {
    //             std::cout << sc_time_stamp() << ": MEM received read" << std::endl;
    //         }

    //         // This simulates memory read/write delay
    //         wait(99);

    //         if (f == FUNC_READ)
    //         {
    //             Port_Data.write( (addr < MEM_SIZE) ? m_data[addr] : 0 );
    //             Port_Done.write( RET_READ_DONE );
    //             wait();
    //             Port_Data.write("ZZZZZZZZ");
    //         }
    //         else
    //         {
    //             if (addr < MEM_SIZE)
    //             {
    //                 m_data[addr] = data;
    //             }
    //             Port_Done.write( RET_WRITE_DONE );
    //         }
    //     }
    // }
};


#endif /* __MEMORY_H__ */
/*
 * File: tutorial.cpp
 *
 * Tutorial implementation for Advances in Computer Architecture Lab session
 * Implements a simple CPU and memory simulation with randomly generated
 * read and write requests
 *
 * Author(s): Michiel W. van Tol, Mike Lankamp, Jony Zhang,
 *            Konstantinos Bousias
 * Copyright (C) 2005-2017 by Computer Systems Architecture group,
 *                            University of Amsterdam
 */

#include <systemc>

using namespace std;
using namespace sc_core; // This pollutes namespace, better: only import what you need.

static const int MEM_SIZE = 512;

SC_MODULE(Memory)
{

public:
    enum Function
    {
        FUNC_READ,
        FUNC_WRITE
    };

    enum RetCode
    {
        RET_READ_DONE,
        RET_WRITE_DONE,
    };

    sc_in<bool>     Port_CLK;
    sc_in<Function> Port_Func;
    sc_in<int>      Port_Addr;
    sc_out<RetCode> Port_Done;
    sc_inout_rv<32> Port_Data;

    SC_CTOR(Memory)
    {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos();
        dont_initialize();

        m_data = new int[MEM_SIZE];
    }

    ~Memory()
    {
        delete[] m_data;
    }

private:
    int* m_data;

    void execute()
    {
        while (true)
        {
            wait(Port_Func.value_changed_event());

            Function f = Port_Func.read();
            int addr   = Port_Addr.read();
            int data   = 0;
            if (f == FUNC_WRITE)
            {
                data = Port_Data.read().to_int();
            }

            // Simulate Memory read/write delay
            wait(100);

            if (f == FUNC_READ)
            {
                Port_Data.write( (addr < MEM_SIZE) ? m_data[addr] : 0 );
                Port_Done.write( RET_READ_DONE );
                wait();
                Port_Data.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
            }
            else
            {
                if (addr < MEM_SIZE)
                    m_data[addr] = data;

                Port_Done.write( RET_WRITE_DONE );
            }
        }
    }
};

SC_MODULE(CPU)
{

public:
    sc_in<bool>                Port_CLK;
    sc_in<Memory::RetCode>     Port_MemDone;
    sc_out<Memory::Function>   Port_MemFunc;
    sc_out<int>                Port_MemAddr;
    sc_inout_rv<32>            Port_MemData;

    SC_CTOR(CPU)
    {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos();
        dont_initialize();
    }

private:
    void execute()
    {
        while(true)
        {
            Memory::Function f = (rand() % 10) < 5 ? Memory::FUNC_READ : Memory::FUNC_WRITE;
            int addr           = (rand() % MEM_SIZE);;
            int data;

            Port_MemAddr.write(addr);
            Port_MemFunc.write(f);

            if (f == Memory::FUNC_WRITE)
            {
                data = rand();
                Port_MemData.write(data);
                wait();
                Port_MemData.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
            }

            wait(Port_MemDone.value_changed_event());

            // Advance one cycle in simulated time
            wait();
        }

    }
};

int sc_main(int argc, char* argv[])
{
    try
    {
        // Instantiate Modules
        Memory mem("main_memory");
        CPU    cpu("cpu");

        // Signals
        sc_buffer<Memory::Function> sigMemFunc;
        sc_buffer<Memory::RetCode>  sigMemDone;
        sc_signal<int>              sigMemAddr;
        sc_signal_rv<32>            sigMemData;

        // The clock that will drive the CPU and Memory
        sc_clock clk;

        // Connecting module ports with signals
        mem.Port_Func(sigMemFunc);
        mem.Port_Addr(sigMemAddr);
        mem.Port_Data(sigMemData);
        mem.Port_Done(sigMemDone);

        cpu.Port_MemFunc(sigMemFunc);
        cpu.Port_MemAddr(sigMemAddr);
        cpu.Port_MemData(sigMemData);
        cpu.Port_MemDone(sigMemDone);

        mem.Port_CLK(clk);
        cpu.Port_CLK(clk);

        cout << "Running (press CTRL+C to interrupt)... " << endl;

        // Start Simulation
        sc_start();
    }

    catch (exception& e)
    {
        cerr << e.what() << endl;
    }
    return 0;
}

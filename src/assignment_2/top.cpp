#include <systemc>
#include "psa.h"
#include "memory.h"
#include "cache.h"
#include "cpu.h"

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

        // Instantiate Modules
        Memory mem("main_memory");
        CPU    cpu("cpu");
        Cache cache("L1_cache");

        // Signals
        sc_buffer<Cache::Function> sigCacheFunc;
        sc_buffer<Cache::FuncRetCode>  sigCacheDone;
        sc_signal<uint32_t>              sigCacheAddr;
        sc_signal_rv<8>            sigCacheData;

        sc_buffer<Memory::Function> sigMemFunc;
        sc_buffer<Memory::RetCode>  sigMemDone;
        sc_signal<uint32_t>              sigMemAddr;
        sc_signal_rv<8>            sigMemData;

        // The clock that will drive the CPU, Memory and cache
        sc_clock clk;

        // Connecting module ports with signals
        cache.port_func(sigCacheFunc);
        cache.port_addr(sigCacheAddr);
        cache.port_data(sigCacheData);
        cache.port_done(sigCacheDone);

        mem.Port_Func(sigMemFunc);
        mem.Port_Addr(sigMemAddr);
        mem.Port_Data(sigMemData);
        mem.Port_Done(sigMemDone);

        cache.port_mem_addr(sigMemAddr);
        cache.port_mem_data(sigMemData);
        cache.port_mem_done(sigMemDone);
        cache.port_mem_func(sigMemFunc);

        cpu.Port_MemFunc(sigCacheFunc);
        cpu.Port_MemAddr(sigCacheAddr);
        cpu.Port_MemData(sigCacheData);
        cpu.Port_MemDone(sigCacheDone);

        mem.Port_CLK(clk);
        cpu.Port_CLK(clk);
        cache.port_clk(clk);

        std::cout << "Running (press CTRL+C to interrupt)... " << std::endl;


        // Start Simulation
        sc_start();

        // Print statistics after simulation finished
        stats_print();
    }

    catch (std::exception& e)
    {
        SC_REPORT_ERROR(LOG_ID, e.what());
    }

    return 0;
}
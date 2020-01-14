/* Questions for lab:
- data, bytes or 32bit words per line?
- if cache line is empty, is it a cache hit or miss?
*/

#include <systemc>
#include <iostream>
#include <array>
#include <memory>
#include <algorithm>
#include <exception>
#include <sstream>

#include "psa.h"

#define MEM_SIZE 512
#define SET_SIZE 8
#define CACHE_LINE_SIZE 32
#define CACHE_SIZE (1 << 15) //32KB
#define TOTAL_SETS ((CACHE_SIZE / CACHE_LINE_SIZE) / SET_SIZE)
#define TOTAL_CACHE_LINES (1 << 10) //1024
#define MEM_LATENCY_CYCLES 100
#define CACHE_LATENCY_CYCLES 1
#define CPUID 0
#define LOG_ID 42

#define UPDATE_TAG(s, l, a)   \
    s.at(l).tag = a.tag        

#define PUT_DATA(s, l, a, d)  \
    s.at(l).line.at(a.byte_offset) = d

#define GET_DATA(s, l, a)     \
    s.at(l).line.at(a.byte_offset)

using namespace sc_core; // This pollutes namespace, better: only import what you need.



typedef union __addr{ //to avoid bitmasking and uses only the 4 bytes of the address, no extra vars needed
    struct {
        uint32_t byte_offset:5;
        uint32_t set_addr:7;
        uint32_t tag:20;
    }__attribute((packed, aligned(4))); //make sure it's no bigger than 4B and aligns to 4B boundary
    uint32_t memory_addr;
} cache_addr_t;

typedef struct __line {
    std::array<uint8_t, CACHE_LINE_SIZE> line;
    struct{
        int32_t tag; //need only 20bits, extra 12 will be useful in the next labs for state.
        uint32_t lru_age; // lru counter. Assuming no cache line will stay for >2^32 accesses as unused.
    };
    inline bool operator<(const struct __line& line) const {
        return this->lru_age < line.lru_age;
    }
    inline bool operator==(const struct __line& line) const {
        return ((this->tag == line.tag) && (this->line == line.line));
    }
} cache_line_t;

typedef std::array<cache_line_t, SET_SIZE> set_t;

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
    sc_in<uint32_t>      Port_Addr;
    sc_out<RetCode> Port_Done;
    sc_inout_rv<8> Port_Data;

    SC_CTOR(Memory)
    {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos(); //sensitive to rising edge of clock
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
                std::cout << sc_time_stamp() << ": MEM received write" << std::endl;
                data = Port_Data.read().to_int();
            }
            else
            {
                std::cout << sc_time_stamp() << ": MEM received read" << std::endl;
            }

            // This simulates memory read/write delay
            wait(99);

            if (f == FUNC_READ)
            {
                Port_Data.write( (addr < MEM_SIZE) ? m_data[addr] : 0 );
                Port_Done.write( RET_READ_DONE );
                wait();
                Port_Data.write("ZZZZZZZZ");
            }
            else
            {
                if (addr < MEM_SIZE)
                {
                    m_data[addr] = data;
                }
                Port_Done.write( RET_WRITE_DONE );
            }
        }
    }
};

SC_MODULE(Cache) {
    private:
        
        std::unique_ptr<std::array<set_t, TOTAL_SETS>> cache;

        void execute();
        unsigned int get_lru_index(const set_t&) const;
        
        inline int8_t is_hit(const cache_addr_t&) const;
        void increment_age(set_t&, cache_line_t&) const;
        __always_inline void evict_entry(const cache_addr_t&, cache_line_t&) const;

    public:
        enum CacheRetCode {
            CACHE_HIT,
            CACHE_MISS
        };

        enum Function {
            FUNC_READ,
            FUNC_WRITE
        };

        enum FuncRetCode {
            RET_READ,
            RET_WRITE
        };

        sc_in<bool> port_clk;
        sc_in<Function> port_func;
        sc_in<uint32_t> port_addr;
        sc_out<FuncRetCode> port_done;
        sc_inout_rv<8> port_data;

        //memory ports
        sc_in<Memory::RetCode>     port_mem_done;
        sc_out<Memory::Function>   port_mem_func;
        sc_out<uint32_t>                port_mem_addr;
        sc_inout_rv<8>            port_mem_data;

        SC_CTOR(Cache) : cache(new std::array<set_t, TOTAL_SETS>)
        {
            if(!cache) {
                throw std::runtime_error("Couldn't initialise cache.");
            }
            for(auto& set : *(cache.get())) {
                for(auto& line : set) {
                    line.line = {};
                    line.tag = -1;
                    line.lru_age = 0;
                }
            }
            SC_THREAD(execute);
            sensitive << port_clk.pos();
            dont_initialize();
            sc_report::register_id(LOG_ID, "[SC_LOG] ");
            sc_report_handler::set_actions (SC_ID_VECTOR_CONTAINS_LOGIC_VALUE_,
                                SC_DO_NOTHING);
            sc_report_handler::set_actions( "/IEEE_Std_1666/deprecated",
                                SC_DO_NOTHING );
            
        }

        ~Cache(){}

        CacheRetCode write(const cache_addr_t&, set_t&, uint8_t) const;
        CacheRetCode read(const cache_addr_t&, set_t&);


};

unsigned int Cache::get_lru_index(const set_t& set) const
{
    unsigned int lru_index = 0;
    cache_line_t highest; //dummy line for comparison
    highest.lru_age = 0;
    highest.line = {};
    highest.tag = 0;

    for(const auto& line : set) {
        if(line.lru_age == 0) {
            lru_index = &line - &set.at(0); //carve out index of line
            goto out;
        }
    }

    for(const auto& line : set) {
        if(highest < line) {
            highest = line;
            lru_index = &line - &set.at(0);
        }
    }

out:
    return lru_index;
}

void Cache::increment_age(set_t& set, cache_line_t& last_used_line) const
{

    for(auto& line : set) {
        if(line == last_used_line) {
            line.lru_age = 1;
        }
        else if(line.lru_age == 0) { //don't increment age of empty cache line
            continue;
        }
        else {
            ++line.lru_age;
        }
    }

}

inline int8_t Cache::is_hit(const cache_addr_t& addr) const
{
    auto cache_p = cache.get();
    int8_t hit = -1; //if no hit, return -1 to evict lru cache line
    std::ostringstream log;
    for(int8_t i = 0; i < SET_SIZE; ++i) {
        log << "Comparing line tag: " << cache_p->at(addr.set_addr).at(i).tag << " -- Addr tag: " << addr.tag;
        SC_REPORT_INFO(LOG_ID, log.str().c_str());
        if((cache_p->at(addr.set_addr).at(i).tag == addr.tag)){
            SC_REPORT_INFO(LOG_ID, "Cache Hit");
            hit = i;
            break;
        }
        log.str("");
        log.clear();
    }

    return hit;
}

__always_inline void Cache::evict_entry(const cache_addr_t& addr, cache_line_t& evict_line) const
{
    for(auto& byte : evict_line.line) {
        byte = 0x42; 
    }
}

Cache::CacheRetCode Cache::write(const cache_addr_t& addr, set_t& set, uint8_t data) const
{
    CacheRetCode retv = CACHE_MISS;
    int8_t line_index = is_hit(addr);
    unsigned int lru_index = get_lru_index(set);

    if(line_index != -1) {
        SC_REPORT_INFO(LOG_ID, "Cache write hit");
        retv = CACHE_HIT;
        PUT_DATA(set, line_index, addr, data);
        increment_age(set, set.at(line_index));
        goto out_hit;
    }
    else {
        SC_REPORT_INFO(LOG_ID, "Cache write miss");
        retv = CACHE_MISS;
        evict_entry(addr, set.at(lru_index));
        PUT_DATA(set, lru_index, addr, data);
        UPDATE_TAG(set, lru_index, addr);
        increment_age(set, set.at(lru_index));
        goto out_miss;
    }

out_hit:
    stats_writehit(CPUID);
    sc_core::wait(CACHE_LATENCY_CYCLES);
    return retv;
out_miss:
    stats_writemiss(CPUID);
    sc_core::wait(MEM_LATENCY_CYCLES);
    return retv;
}


Cache::CacheRetCode Cache::read(const cache_addr_t& addr, set_t& set) 
{
    CacheRetCode retv = CACHE_MISS;
    int8_t line_index = 0;
    unsigned int lru_index = get_lru_index(set);
    uint8_t data;

    if((line_index = is_hit(addr)) != -1) {
        SC_REPORT_INFO(LOG_ID, "Cache read hit");
        retv = CACHE_HIT;
        data = GET_DATA(set, line_index, addr);
        port_data.write(data);
        increment_age(set, set.at(line_index));
        goto out_hit;
    }
    else {
        SC_REPORT_INFO(LOG_ID, "Cache read miss");
        retv = CACHE_MISS;
        evict_entry(addr, set.at(lru_index));
        data = GET_DATA(set, lru_index, addr);
        UPDATE_TAG(set, lru_index, addr);
        port_data.write(data);
        increment_age(set, set.at(lru_index));
        goto out_miss;
    }


out_hit:
    stats_readhit(CPUID);
    sc_core::wait(CACHE_LATENCY_CYCLES);
    return retv;
out_miss:
    stats_readmiss(CPUID);
    sc_core::wait(MEM_LATENCY_CYCLES);
    return retv;

}

void Cache::execute()
{   
    auto cache_p = cache.get();
    cache_addr_t addr;
    Function func;
    uint8_t data;
    std::ostringstream log;
    
    while(1) {
        wait(port_func.value_changed_event());
        SC_REPORT_INFO(LOG_ID, "\nReceived Function Signal!\n");
        func = port_func.read();
        addr.memory_addr = port_addr.read();
        data = 0;
        log << "Tag: " << addr.tag << " -- Set: " << addr.set_addr << " -- Offset: " << addr.byte_offset;
        SC_REPORT_INFO(LOG_ID, log.str().c_str());
        if(func == FUNC_WRITE) {
            SC_REPORT_INFO(LOG_ID, "Executing Cache Write");
            data = static_cast<uint8_t>(port_data.read().to_int());
            this->write(addr, cache_p->at(addr.set_addr), data);
            port_done.write(RET_WRITE);
        }
        else if(func == FUNC_READ) {
            SC_REPORT_INFO(LOG_ID, "Executing Cache Read");
            this->read(addr, cache_p->at(addr.set_addr));
            port_done.write(RET_READ);
        }
        else {
            throw std::runtime_error("Unknown Function");
        }
        log.str("");
        log.clear();
    }

}



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
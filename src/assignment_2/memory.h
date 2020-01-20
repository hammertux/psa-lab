#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <systemc>


#define MEM_SIZE 512

using namespace sc_core;

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


#endif /* __MEMORY_H__ */
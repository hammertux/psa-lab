#ifndef MEMORY_H
#define MEMORY_H

#include <systemc.h>
#include <iostream>

#include "bus_slave_if.h"
#include "helpers.h"

class Memory : public bus_slave_if, public sc_module {

public:

    SC_CTOR(Memory) {
        // nothing to do here right now.
    }

    ~Memory() {
        // nothing to do here right now.
    }

    int read(uint32_t addr) {
        assert((addr & 0x3) == 0);
        log(name(), "read from address", addr);
        wait(100);
        return 0;
    }

    int write(uint32_t addr) {
        assert((addr & 0x3) == 0);
        log(name(), "write to address", addr);
        wait(100);
        return 0;
    }
};
#endif

#ifndef CACHE_H
#define CACHE_H

#include <systemc.h>
#include <iostream>

#include "cpu_cache_if.h"
#include "Memory.h"
#include "helpers.h"

// Class definition without the SC_ macro because we implement the
// cpu_cache_if interface.
class Cache : public cpu_cache_if , public sc_module {

public:
    sc_port<bus_slave_if> memory;

    // cpu_cache interface methods.
    int cpu_read(uint32_t addr);
    int cpu_write(uint32_t addr);

    // Constructor without SC_ macro.
    Cache(sc_module_name name_, int id_) : sc_module(name_), id(id_) {
        // Passive for now, just handle cpu requests.
    }

    ~Cache() {
    }

private:
    int id;
};

#endif

#include <systemc.h>

#ifndef BUS_SLAVE_IF_H
#define BUS_SLAVE_IF_H

class bus_slave_if : public virtual sc_interface {
    public:
    virtual int read(uint32_t addr) = 0;
    virtual int write(uint32_t addr) = 0;
};

#endif

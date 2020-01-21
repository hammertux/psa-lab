#ifndef __BUS_SLAVE_IF_H__
#define __BUS_SLAVE_IF_H__

#include <systemc>
#include "util.h"

class Bus_slave_if : public virtual sc_interface {
    public:
        virtual int read(const cache_addr_t&) = 0;
        virtual int write(const cache_addr_t&, uint8_t data) = 0;

};




#endif /* __BUS_SLAVE_IF_H__ */
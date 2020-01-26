#ifndef __BUS_IF_H__
#define __BUS_IF_H__

#include <systemc>
#include "util.h"


class Bus_if : public virtual sc_interface {
    public:
        virtual void read(uint16_t, uint32_t) = 0;
        virtual void write(uint16_t, uint32_t, uint8_t) = 0;

        virtual ~Bus_if() {}
};





#endif /* __BUS_IF_H__ */
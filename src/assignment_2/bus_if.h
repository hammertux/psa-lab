#ifndef __BUS_IF_H__
#define __BUS_IF_H__

#include <systemc>


class Bus_if : public virtual sc_interface {
    public:
        virtual bool read(int addr) = 0;
        virtual bool write(int addr, int data) = 0;
};





#endif /* __BUS_IF_H__ */
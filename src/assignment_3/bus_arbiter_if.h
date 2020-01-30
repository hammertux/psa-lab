#ifndef __BUS_ARBITER_IF_H__
#define __BUS_ARBITER_IF_H___

#include <systemc>
#include "util.h"

class Bus_arbiter_if : public virtual sc_core::sc_interface {

    public:
        virtual std::shared_ptr<bus_sig_t> arbitrate(std::vector<bus_sig_t>&) = 0;

};



#endif /* __BUS_ARBITER_IF_H__ */
#ifndef __BUS_ARBITER_H__
#define __BUS_ARBITER_H__

#include "bus_arbiter_if.h"
#include "util.h"


class Arbiter : public virtual Bus_arbiter_if, public sc_core::sc_module {

    public:
        Arbiter(sc_core::sc_module_name __name) : sc_core::sc_module(__name) {}
        std::shared_ptr<bus_sig_t> arbitrate(std::vector<bus_sig_t>&) override;

};



#endif /* __BUS_ARBITER_H__ */
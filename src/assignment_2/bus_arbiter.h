#ifndef __BUS_ARBITER_H__
#define __BUS_ARBITER_H__

#include "bus_arbiter_if.h"
#include "util.h"


class Arbiter : public virtual Bus_arbiter_if, public sc_core::sc_module {
    private:
        uint64_t total_requests_issued;
        sc_time cumulative_time;

    public:
        Arbiter(sc_core::sc_module_name __name) : sc_core::sc_module(__name), cumulative_time(1, SC_NS) {
            total_requests_issued = 0;
        }
        std::shared_ptr<bus_sig_t> arbitrate(std::vector<bus_sig_t>&) override;
        __always_inline const uint64_t get_total_reqs() const {
            return total_requests_issued;
        }

        __always_inline const sc_time get_cumulative_time() const {
            return cumulative_time;
        }

};



#endif /* __BUS_ARBITER_H__ */
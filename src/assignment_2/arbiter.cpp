#include "bus_arbiter.h"

std::shared_ptr<bus_sig_t> Arbiter::arbitrate(std::vector<bus_sig_t>& reqs)
{
    auto it = reqs.begin();
    std::shared_ptr<bus_sig_t> req;
    if(reqs.empty()) {
        //std::cout << "ARBITER EMPTY" << std::endl;
        return nullptr;
        // throw std::runtime_error("Arbiter empty queue");
    }
    for(it = reqs.begin(); it != reqs.end(); ++it) {
        if(it->req_status == REQ_CACHE_PROCESSING) {
            req = std::make_shared<bus_sig_t>(*it);
            reqs.erase(it);
            return req;
        }
    }
    req = std::make_shared<bus_sig_t>(reqs.front());
    reqs.erase(reqs.begin());
    // for(auto& i : reqs) {
    //     i.reset();
    // }
    //std::cout << "ARBITER REQ = " << *(req.get()) << std::endl;
    return req;
    
}
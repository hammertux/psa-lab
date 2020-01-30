#include "bus_arbiter.h"

std::shared_ptr<bus_sig_t> Arbiter::arbitrate(std::vector<bus_sig_t>& reqs)
{
    auto it = reqs.begin();
    std::shared_ptr<bus_sig_t> req;
    if(reqs.empty()) {
        return nullptr;
    }
    for(it = reqs.begin(); it != reqs.end(); ++it) {
        if(it->req_status == REQ_CACHE_PROCESSING) {
            req = std::make_shared<bus_sig_t>(*it);
            reqs.erase(it);
            cumulative_time += (sc_time_stamp() - req->time_of_issue_to_bus);
            ++total_requests_issued;
            return req;
        }
    }
    req = std::make_shared<bus_sig_t>(reqs.front());
    reqs.erase(reqs.begin());
    cumulative_time += (sc_time_stamp() - req->time_of_issue_to_bus);
    ++total_requests_issued;
    return req;
    
}
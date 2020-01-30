#include "bus.h"


void Bus::execute()
{
    
    std::vector<bus_sig_t> in_flight_reqs;
    auto current_p = current_req.get();
    MEM_REQ_STATUS mem_status;
    bus_sig_t tmp;
    while(1) {
        wait();
        if(requests.size() > last_queue_size || !(old_tail == *(requests.back().get()))) {
            std::cout << "Queue enlarged" << std::endl;
            port_c2c_inout->write(*(requests.back().get())); //check if other caches have it
        }
        last_queue_size = requests.size();
        old_tail = *(requests.back().get());
        if(!current_req) {
            for(const auto& req : requests) {
                auto req_p = req.get();
                if(req_p->req_status == REQ_CACHE_QUEUED) {
                    in_flight_reqs.push_back(*req_p);
                }
            }
            current_req = arbiter->arbitrate(in_flight_reqs);
        }

        current_p = current_req.get();
        if(current_req) {
            for(auto it = requests.begin(); it != requests.end();) {
                if(*(it->get()) == *current_p) {
                    it->reset();
                    it = requests.erase(it);
                }
                else {
                    ++it;
                }
            }
            current_p->req_status = REQ_CACHE_PROCESSING;
            if(current_p->is_c2c) {
                current_p->req_status = REQ_CACHE_DONE;
                port_bus_inout->write(*current_p);
                current_req.reset();
            }
            else {
                mem_status = (current_p->b == BUS_READ) ? memory->read(current_p->addr) : memory->write(current_p->addr, RANDOM_DATA);
                if(mem_status == REQ_MEM_DONE) {
                    current_p->req_status = REQ_CACHE_DONE;
                    port_bus_inout->write(*current_p);
                    current_req.reset();
                }
            }
        }
        in_flight_reqs.clear();
    }
    
}

void Bus::read(uint16_t id, uint32_t addr)
{
    std::shared_ptr<bus_sig_t> sig;
    sig = std::make_shared<bus_sig_t>(BUS_READ, addr, id);
    sig.get()->req_status = REQ_CACHE_QUEUED;
    sig.get()->time_of_issue_to_bus = sc_time_stamp();
    for(const auto& req : requests) {
        if(*(sig.get()) == *(req.get())) {
            sig.reset();
            return;
        }
    }
    requests.push_back(sig);
}

void Bus::write(uint16_t id, uint32_t addr, uint8_t data)
{
    std::shared_ptr<bus_sig_t> sig;
    sig = std::make_shared<bus_sig_t>(BUS_WRITE, addr, id);
    sig.get()->req_status = REQ_CACHE_QUEUED;
    sig.get()->time_of_issue_to_bus = sc_time_stamp();
    for(const auto& req : requests) {
        if(*(sig.get()) == *(req.get())) {
            std::cout << "Exists already" << std::endl;
            sig.reset();
            return;
        }
    }
    requests.push_back(sig);
}

void Bus::cache_to_cache(uint16_t id, uint32_t addr)
{
    bus_sig_t sig_rd = bus_sig_t(BUS_READ, addr, id);
    bus_sig_t sig_wr = bus_sig_t(BUS_WRITE, addr, id);
    std::cout << "C2C called " << std::endl;

    for(auto it = requests.begin(); it != requests.end(); ++it) {
        if(*(it->get()) == sig_rd) {
            *it = std::make_shared<bus_sig_t>(sig_rd);
            it->get()->is_c2c = true;
        }
        else if(*(it->get()) == sig_wr) {
            *it = std::make_shared<bus_sig_t>(sig_wr);
            it->get()->is_c2c = true;
        }
    }
}


#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>
#include <array>
#include <exception>
#include <iostream>
#include <memory>
#include "psa.h"

#define SET_SIZE 8
#define CACHE_LINE_SIZE 32
#define CACHE_SIZE (1 << 15) //32KB
#define TOTAL_SETS ((CACHE_SIZE / CACHE_LINE_SIZE) / SET_SIZE)
#define TOTAL_CACHE_LINES (1 << 10) //1024
#define MEM_LATENCY_CYCLES 100
#define CACHE_LATENCY_CYCLES 1
#define CPUID 0
#define LOG_ID 42

#define RANDOM_DATA 0x42

using namespace sc_core;

typedef union __addr{ //to avoid bitmasking and uses only the 4 bytes of the address, no extra vars needed
    struct {
        uint32_t byte_offset:5;
        uint32_t set_addr:7;
        uint32_t tag:20;
    }__attribute((packed, aligned(4))); //make sure it's no bigger than 4B and aligns to 4B boundary
    uint32_t memory_addr;
} cache_addr_t;

typedef struct __line {
    std::array<uint8_t, CACHE_LINE_SIZE> line;
    struct{
        uint32_t valid:1;
        uint32_t tag:30; //need only 20bits, extra 12 will be useful in the next labs for state.
        uint32_t lru_age; // lru counter. Assuming no cache line will stay for >2^32 accesses as unused.
    };
    inline bool operator<(const struct __line& line) const {
        return this->lru_age < line.lru_age;
    }
    inline bool operator==(const struct __line& line) const {
        return ((this->tag == line.tag) && (this->line == line.line));
    }
} cache_line_t;

typedef std::array<cache_line_t, SET_SIZE> set_t;

enum BUS_OP {
        BUS_WRITE,
        BUS_READ
};

enum CACHE_REQ_STATUS {
    REQ_CACHE_QUEUED,
    REQ_CACHE_PROCESSING,
    REQ_CACHE_DONE
};

enum MEM_REQ_STATUS {
    REQ_MEM_PROCESSING,
    REQ_MEM_DONE
};

struct bus_sig_t{
    
    BUS_OP b;
    uint32_t addr;
    uint16_t id;
    CACHE_REQ_STATUS req_status;
    sc_time time_of_issue_to_bus;

    bus_sig_t() : addr(0){}
    bus_sig_t(BUS_OP _b, uint32_t _addr, uint16_t _id) : b(_b), addr(_addr), id(_id) {}
    
    inline bool operator ==(const bus_sig_t& sig) {
        return (this->addr == sig.addr) && (this->b == sig.b) && (this->id == sig.id);
    }

    inline bus_sig_t operator =(const bus_sig_t& sig) {
        this->b = sig.b;
        this->addr = sig.addr;
        this->id = sig.id;
        this->req_status = sig.req_status;
        return *this;
    }

    friend void sc_trace(sc_trace_file *tf, const bus_sig_t& sig, const std::string& name) {
        sc_trace(tf, sig.b, name + ".BUS_OP");
        sc_trace(tf, sig.addr, name + ".addr");
        sc_trace(tf, sig.id, name + ".id");
    }

    inline friend std::ostream& operator <<(std::ostream& os,  bus_sig_t const & sig) {
      os << "(" << sig.b << ", " << std::boolalpha << sig.addr << ", " << sig.id << ", " << sig.req_status << ")";
      return os;
    }

};

#endif /* __UTIL_H__ */
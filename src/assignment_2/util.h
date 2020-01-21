#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>
#include <array>
#include <exception>

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

typedef std::pair<uint32_t, uint16_t> addr_id_pair_t;


#endif /* __UTIL_H__ */
#include <assert.h>
#include <systemc.h>

#include "Cache.h"
#include "psa.h"

/* cpu_cache_if interface method
 * Called by CPU.
 */
int Cache::cpu_read(uint32_t addr) {
    int hit = rand()%2; // cache not modeled.
    int dirty = rand()%2;

    if (hit) { // Hit!
        log(name(), "read hit on address", addr);
        stats_readhit(id);
        return 0; // succes
    }

    log(name(), "read miss on address", addr);
    stats_readmiss(id);
    if (dirty) {
        // Write back
        log(name(), "replacement write back");
        memory->write(128); // cache not modeled so address unknown
    }

    // Cache miss, read cache line from memory.
    log(name(), "read address", addr);
    memory->read(addr);
    return 0; // Done, return value 0 signals succes.
}

/* cpu_cache_if interface method
 * Called by CPU.
 */
int Cache::cpu_write(uint32_t addr) {
    int hit = rand()%2; // cache not modeled.
    int dirty = rand()%2;

    if (hit) { // Hit!
        log(name(), "write hit on address", addr);
        stats_writehit(id);
        // write back cache, so don't write through to memory.
        return 0;
    }

    log(name(), "write miss on address", addr);
    stats_writemiss(id);
    if (dirty) { // cache line is dirty.
        // Write back cache line to memory
        log(name(), "replacement write back");
        memory->write(128); // cache not modeled so address unknown
    }
    // Read complete cache line from memory.
    log(name(), "read address", addr);
    memory->read(addr);

    return 0; // indicates succes.
}

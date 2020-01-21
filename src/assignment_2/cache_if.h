#ifndef __CACHE_IF_H__
#define __CACHE_IF_H__

#include <systemc>
#include "util.h"

using namespace sc_core;



class Cache_if : public virtual sc_interface {
    public:
        virtual int read(const cache_addr_t&) = 0;
        virtual int write(const cache_addr_t&, const uint8_t) = 0;

        virtual ~Cache_if() {};
};



#endif /* __CACHE_IF_H__ */
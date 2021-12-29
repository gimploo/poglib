#pragma once
#include "../../basic.h"


typedef struct c_lifespan_t c_lifespan_t ;

c_lifespan_t        c_lifespan_init(u64 total);




#ifndef IGNORE_C_LIFESPAN_IMPLEMENTATION

struct c_lifespan_t {

    u64 remaining;
    u64 total;

};


c_lifespan_t c_lifespan_init(u64 total)
{
    return (c_lifespan_t ) {
        .remaining = total,
        .total = total
    };
}

#endif

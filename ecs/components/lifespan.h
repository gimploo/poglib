#pragma once
#include "../../basic.h"


typedef struct c_lifespan_t c_lifespan_t ;

c_lifespan_t * c_lifespan_init(i64 total);




#ifndef IGNORE_C_LIFESPAN_IMPLEMENTATION

struct c_lifespan_t {

    bool is_alive;
    i64 remaining;
    i64 total;

    void (*update)(c_lifespan_t *, u32 factor);

};

void __c_lifespan_update(c_lifespan_t *cmp, u32 factor)
{
    if(cmp->remaining <= 0)  {
        cmp->is_alive = false;
        return;
    }

    cmp->remaining -= factor;
}

c_lifespan_t * c_lifespan_init(i64 total)
{
    c_lifespan_t *o = (c_lifespan_t *)calloc(1, sizeof(c_lifespan_t ));
    assert(o);

    *o =  (c_lifespan_t ) {
        .is_alive = true,
        .remaining = (i64 )total,
        .total = total,
        .update = __c_lifespan_update 
    };
    return o;
}

#endif

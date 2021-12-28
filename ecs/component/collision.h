#pragma once
#include "../../basic.h"


typedef struct c_collision2d_t c_collision2d_t ;

c_collision2d_t     c_collision_init(f32 radius);





#ifndef IGNORE_C_COLLISION_IMPLEMENTATION

struct c_collision2d_t {
    f32 radius;
};


c_collision2d_t c_collision_init(f32 radius)
{
    return (c_collision2d_t ) {
        .radius = radius
    };
}

#endif

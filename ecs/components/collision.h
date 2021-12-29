#pragma once
#include "../../basic.h"
#include "../../simple/gl/types.h"


typedef struct c_boxcollider2d_t c_boxcollider2d_t ;

c_boxcollider2d_t     c_boxcollider2d_init(f32 radius);





#ifndef IGNORE_C_COLLISION_IMPLEMENTATION

struct c_boxcollider2d_t {
    f32 radius;
};


c_boxcollider2d_t c_collision_init(f32 radius)
{
    return (c_boxcollider2d_t ) {
        .radius = radius
    };
}

#endif

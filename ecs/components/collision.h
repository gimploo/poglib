#pragma once
#include "../../basic.h"
#include "../../simple/gl/types.h"


typedef struct c_boxcollider2d_t c_boxcollider2d_t ;

c_boxcollider2d_t     c_boxcollider2d_init(vec3f_t *pos, f32 side);





#ifndef IGNORE_C_COLLISION_IMPLEMENTATION

struct c_boxcollider2d_t {

    f32 side;
    vec3f_t *position;

};


c_boxcollider2d_t c_collision_init(vec3f_t *pos, f32 side)
{
    return (c_boxcollider2d_t ) {
        .side = side,
        .position = pos
    };
}

#endif

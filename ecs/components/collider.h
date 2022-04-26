#pragma once
#include "../../basic.h"
#include "../components/transform.h"


typedef struct c_boxcollider2d_t c_boxcollider2d_t ;



c_boxcollider2d_t c_boxcollider2d(vec2f_t side);




#ifndef IGNORE_C_COLLIDER_IMPLEMENTATION

struct c_boxcollider2d_t {

    vec3f_t centerpos;
    vec2f_t halfside;
    vec2f_t side;

    vec2f_t __prev_frame_overlap;
};


c_boxcollider2d_t c_boxcollider2d(vec2f_t side)
{
    c_boxcollider2d_t o =  (c_boxcollider2d_t ) {
        .centerpos   = vec3f(0.0f),
        .halfside    = (vec2f_t ){side.cmp[X]/2.0f, side.cmp[Y]/2.0f},
        .side        = side,
        .__prev_frame_overlap = vec2f(0.0f)
    };

    return o;
}


#endif

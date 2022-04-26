#pragma once
#include "../../math/la.h"
#include "../components/transform.h"


//TODO: Outline, thickness

typedef enum c_shape_type {

    TRIANGLE = 0,
    SQUARE,
    CIRCLE,

    CST_COUNT

} c_shape_type;



typedef struct c_shape2d_t c_shape2d_t ;


#define c_shape2d(TYPE, RADIUS, FILL) __impl_c_shape2d_init(TYPE, (RADIUS), (FILL))







#ifndef IGNORE_C_SHAPE_IMPLEMENTATION


struct c_shape2d_t {

    c_shape_type    type;
    f32             radius;
    vec4f_t         fill;
    u32             nvertex;
};



c_shape2d_t __impl_c_shape2d_init(c_shape_type type, f32 radius, vec4f_t fill)
{
    c_shape2d_t o = (c_shape2d_t ){
        .type   = type,
        .radius = radius,
        .fill   = fill,
    };

    switch(type)
    {
        case TRIANGLE:
            o.nvertex = 3;
        break;

        case SQUARE:
            o.nvertex = 4;
        break;

        case CIRCLE:
            o.nvertex = MAX_VERTICES_PER_CIRCLE;
        break;

        default: eprint("type not accounted for");
    }

    return o;
}


#endif 

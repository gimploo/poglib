#pragma once
#include "../../math/la.h"
#include "../../simple/gl/types.h"
#include "../components/transform.h"


//TODO: Outline, thickness

typedef enum c_shape_type {

    CST_TRIANGLE = 0,
    CST_SQUARE,
    CST_CIRCLE,

    CST_COUNT

} c_shape_type;



typedef struct c_shape2d_t c_shape2d_t ;


#define c_shape2d_init(TYPE, RADIUS, FILL) __impl_c_shape2d_init(CST_##TYPE, (RADIUS), (FILL))







#ifndef IGNORE_C_SHAPE_IMPLEMENTATION


struct c_shape2d_t {

    c_shape_type    type;
    f32             radius;
    vec4f_t         fill;
};



c_shape2d_t * __impl_c_shape2d_init(c_shape_type type, f32 radius, vec4f_t fill)
{
    c_shape2d_t *o = (c_shape2d_t *)calloc(1, sizeof(c_shape2d_t));
    *o = (c_shape2d_t ){
        .type   = type,
        .radius = radius,
        .fill   = fill,
    };

    return o;
}


#endif 

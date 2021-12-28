#pragma once
#include "../../math/la.h"


//TODO: Outline, thickness

typedef struct c_shape_t c_shape_t ;



c_shape_t c_shape_init(u64 sides, f32 radius, vec4f_t fill, vec3f_t origin);





#ifndef IGNORE_C_SHAPE_IMPLEMENTATION


struct c_shape_t {

    f32 radius;
    vec4f_t fill;
    u64 sides;

};


c_shape_t c_shape_init(u64 sides, f32 radius, vec4f_t fill)
{
    return (c_shape_t ) {
        .radius = radius,
        .fill = fill,
    };
}

#endif 

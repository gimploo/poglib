#pragma once 
#include "../../simple/gl/types.h"



typedef struct c_transform_t c_transform_t ;


c_transform_t c_transform_init(vec3f_t pos, vec3f_t vec, f32 angle);







#ifndef IGNORE_C_TRANSFORM_IMPLEMENTATION


struct c_transform_t {

    vec3f_t position;
    vec3f_t scale;
    f32 angle;

};

c_transform_t c_transform_init(vec3f_t pos, vec3f_t vec, f32 angle)
{
    return (c_transform_t ) {
        .position = pos,
        .scale = vec,
        .angle = angle
    };
}

#endif

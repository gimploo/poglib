#pragma once 
#include "../../math/la.h"



typedef struct c_transform_t c_transform_t ;


c_transform_t c_transform_init(vec3f_t pos, vec3f_t vec, f32 angle);







#ifndef IGNORE_C_TRANSFORM_IMPLEMENTATION


struct c_transform_t {

    vec3f_t position;
    vec3f_t speed;
    f32 angle;
};

c_transform_t c_transform_init(vec3f_t pos, vec3f_t vec, f32 angle)
{
    return (c_transform_t ) {
        pos, vec, angle
    };
}

#endif

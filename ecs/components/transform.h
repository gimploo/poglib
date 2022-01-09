#pragma once 
#include "../../simple/gl/types.h"



typedef struct c_transform_t c_transform_t ;


c_transform_t * c_transform_init(vec3f_t pos, f32 angle, vec3f_t velocity);







#ifndef IGNORE_C_TRANSFORM_IMPLEMENTATION


struct c_transform_t {

    vec3f_t position;
    vec3f_t velocity;
    f32     angle;

    void (*update)(c_transform_t *);

};

void __c_transform_update(c_transform_t *transform)
{
    transform->position = vec3f_add(transform->position, transform->velocity);
}

c_transform_t * c_transform_init(vec3f_t pos, f32 angle, vec3f_t velocity)
{
    c_transform_t * o = (c_transform_t *)calloc(1, sizeof(c_transform_t ));

    velocity.cmp[X] *= (f32)cos(angle);
    velocity.cmp[Y] *= (f32)sin(angle);

    *o = (c_transform_t ) {
        .position   = pos,
        .velocity   = velocity,
        .angle      = angle,
        .update     = __c_transform_update
    };
    return o;
}

#endif

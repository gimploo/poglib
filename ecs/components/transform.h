#pragma once 
#include "../../simple/gl/types.h"



typedef struct c_transform_t c_transform_t ;


c_transform_t * c_transform_init(vec3f_t pos, vec3f_t velocity, f32 angular_speed, f32 angular_radians);





#ifndef IGNORE_C_TRANSFORM_IMPLEMENTATION


struct c_transform_t {

    vec3f_t position;
    vec3f_t prev_position;
    vec3f_t velocity;

    f32 angular_speed;
    f32 angular_radians;

    void (*update)(c_transform_t *);

};


void __c_transform_update(c_transform_t *transform)
{
    transform->prev_position = transform->position;

    // Translation
    transform->position = vec3f_add(transform->position, transform->velocity);
    
    //Rotation
}

c_transform_t * c_transform_init(vec3f_t pos, vec3f_t velocity, f32 angular_speed, f32 angular_radians)
{
    c_transform_t * o = (c_transform_t *)calloc(1, sizeof(c_transform_t ));

    *o = (c_transform_t ) {
        .position           = pos,
        .prev_position      = pos,
        .velocity           = velocity,
        .angular_speed      = angular_speed,
        .angular_radians    = angular_radians,
        .update             = __c_transform_update
    };
    return o;
}

#endif

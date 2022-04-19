#pragma once 
#include "../../simple/gl/types.h"


typedef struct c_transform_t {

    vec3f_t position;
    vec3f_t prev_position;
    vec3f_t velocity;

    f32 angular_speed;
    f32 angular_radians;

    void (*update)(struct c_transform_t *);

} c_transform_t ;


c_transform_t c_transform(vec3f_t pos, vec3f_t velocity, f32 angular_speed, f32 angular_radians);


#ifndef IGNORE_C_TRANSFORM_IMPLEMENTATION

void __c_transform_update(c_transform_t *transform)
{
    transform->prev_position = transform->position;

    // Translation
    transform->position = vec3f_add(transform->position, transform->velocity);
    
    //Rotation
}

c_transform_t c_transform(vec3f_t pos, vec3f_t velocity, f32 angular_speed, f32 angular_radians)
{
    return (c_transform_t ) {
        .position           = pos,
        .prev_position      = pos,
        .velocity           = velocity,
        .angular_speed      = angular_speed,
        .angular_radians    = angular_radians,
        .update             = __c_transform_update
    };
}

#endif

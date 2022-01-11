#pragma once 
#include "../../simple/gl/types.h"



typedef struct c_transform_t c_transform_t ;


c_transform_t * c_transform_init(vec3f_t pos, f32 speed, f32 radians, f32 angular_speed, f32 angular_radians);
#define c_transform_update(PCTRANSFORM) (PCTRANSFORM)->update(PCTRANSFORM)







#ifndef IGNORE_C_TRANSFORM_IMPLEMENTATION


struct c_transform_t {

    vec3f_t position;
    f32     speed;
    f32     angle;
    vec3f_t velocity;

    f32 angular_speed;
    f32 angular_radians;

    void (*update)(c_transform_t *);

};


void __c_transform_update(c_transform_t *transform)
{
    transform->position = vec3f_add(transform->position, transform->velocity);
}

c_transform_t * c_transform_init(vec3f_t pos, f32 speed, f32 radians, f32 angular_speed, f32 angular_radians)
{
    c_transform_t * o = (c_transform_t *)calloc(1, sizeof(c_transform_t ));

    vec3f_t velocity = {
         .cmp[X] = speed * (f32)cos(radians),
         .cmp[Y] = speed * (f32)sin(radians),
         .cmp[Z] = 0.0f   
    };

    *o = (c_transform_t ) {
        .position           = pos,
        .speed              = speed,
        .angle              = radians,
        .velocity           = velocity,
        .angular_speed      = angular_speed,
        .angular_radians    = angular_radians,
        .update             = __c_transform_update
    };
    return o;
}

#endif

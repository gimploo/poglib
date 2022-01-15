#pragma once
#include "../../basic.h"
#include "../../simple/gl/types.h"
#include "../components/transform.h"


typedef struct c_boxcollider2d_t c_boxcollider2d_t ;



c_boxcollider2d_t *     c_boxcollider2d_init(vec3f_t pos, f32 side);




#ifndef IGNORE_C_COLLIDER_IMPLEMENTATION

struct c_boxcollider2d_t {

    f32 side;
    vec3f_t position;

    quadf_t __cache;
    void (*update) (struct c_boxcollider2d_t *);
};

void __c_box_collider2d_update(c_boxcollider2d_t *cmp)
{
    cmp->__cache = quadf_init( 
            cmp->position,
            cmp->side, cmp->side);
}


c_boxcollider2d_t * c_boxcollider2d_init(vec3f_t pos, f32 side)
{
    c_boxcollider2d_t *o = (c_boxcollider2d_t *)calloc(1, sizeof(*o));
    assert(o);

    *o =  (c_boxcollider2d_t ) {
        .side = side,
        .position = pos,
        .update = __c_box_collider2d_update
    };

    __c_box_collider2d_update(o);

    return o;
}


#endif

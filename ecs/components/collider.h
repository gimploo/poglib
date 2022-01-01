#pragma once
#include "../../basic.h"
#include "../../simple/gl/types.h"
#include "../components/transform.h"


typedef struct c_boxcollider2d_t c_boxcollider2d_t ;



c_boxcollider2d_t     c_boxcollider2d_init(c_transform_t *t, f32 side);
#define               c_boxcollider2d_update(PBOX) (PBOX)->update(PBOX)




#ifndef IGNORE_C_COLLIDER_IMPLEMENTATION

struct c_boxcollider2d_t {

    f32 side;
    vec3f_t *position;

    quadf_t __cache;
    void (*update) (struct c_boxcollider2d_t *);
};

void __c_box_collider2d_update(c_boxcollider2d_t *cmp)
{
    cmp->__cache = quadf_init( 
            (vec2f_t ){ cmp->position->cmp[X], cmp->position->cmp[Y] }, 
            cmp->side, cmp->side);
}


c_boxcollider2d_t c_collision_init(c_transform_t *t, f32 side)
{
    return (c_boxcollider2d_t ) {
        .side = side,
        .position = &t->position,
        .update = __c_box_collider2d_update
    };
}


#endif

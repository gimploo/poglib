#pragma once
#include "../components/collider.h"

bool collision2d_check_collision_by_AABB(c_boxcollider2d_t *a, c_boxcollider2d_t *b);


#ifndef IGNORE_COLLISION2D_SYSTEM_IMPLEMENTATION

bool collision2d_check_collision_by_AABB(c_boxcollider2d_t *a, c_boxcollider2d_t *b)
{
    assert(a);
    assert(b);

    vec2f_t delta = {
        .cmp = {
            [X] = fabs(a->centerpos.cmp[X] - b->centerpos.cmp[X]), 
            [Y] = fabs(a->centerpos.cmp[Y] - b->centerpos.cmp[Y])
        }
    };

    f32 ox = a->halfside.cmp[X] + b->halfside.cmp[X] - delta.cmp[X];
    f32 oy = a->halfside.cmp[Y] + b->halfside.cmp[Y] - delta.cmp[Y];

    a->__prev_frame_overlap = (vec2f_t ){ox, oy};
    b->__prev_frame_overlap = (vec2f_t ){ox, oy};

    return ((ox > 0) && (oy > 0));
}

bool collision2d_check_out_of_screen(c_boxcollider2d_t *a)
{
    return (a->centerpos.cmp[X] > 1.0f 
            || a->centerpos.cmp[X] < -1.0f
            || a->centerpos.cmp[Y] > 1.0f
            || a->centerpos.cmp[Y] < -1.0f);
}


#endif

#pragma once
#include "../components/collider.h"


bool            collision2d_check_collision_by_AABB(c_boxcollider2d_t *a, c_boxcollider2d_t *b);



#ifndef IGNORE_COLLISION2D_SYSTEM_IMPLEMENTATION

bool collision2d_check_collision_by_AABB(c_boxcollider2d_t *a, c_boxcollider2d_t *b)
{
    // AABB - AABB collision
    //
    quadf_t *left = &a->__cache;
    quadf_t *right = &b->__cache;
    
    bool x_axis_collision = 
        (left->vertex[1].cmp[X] >= right->vertex[0].cmp[X]) 
        && (right->vertex[1].cmp[X] >= left->vertex[0].cmp[X]);

    bool y_axis_collision = 
        (left->vertex[0].cmp[Y] >= right->vertex[3].cmp[Y]) 
        && (right->vertex[0].cmp[Y] >= left->vertex[3].cmp[Y]);

    return (x_axis_collision && y_axis_collision);
}

#endif

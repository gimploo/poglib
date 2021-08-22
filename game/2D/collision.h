#pragma once 

#include "../../math/shapes.h"

bool collision_quad_check_by_AABB(quadf_t left, quadf_t right)
{
    // AABB - AABB collision
    
    bool x_axis_collision = 
        (left.vertex[1].cmp[X] >= right.vertex[0].cmp[X]) 
        && (right.vertex[1].cmp[X] >= left.vertex[0].cmp[X]);

    bool y_axis_collision = 
        (left.vertex[0].cmp[Y] >= right.vertex[3].cmp[Y]) 
        && (right.vertex[0].cmp[Y] >= left.vertex[3].cmp[Y]);

    return x_axis_collision && y_axis_collision;
}



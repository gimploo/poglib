#pragma once 

#include "../../math/shapes.h"


typedef struct boxcollider2d_t boxcollider2d_t;

boxcollider2d_t     boxcollider2d_init(quadf_t *vertices);
#define             boxcollider2d_update(PBOX1, PBOX2)      __impl_boxcollider2d_collision_using_AABB((PBOX1), (PBOX2))
#define             boxcollider2d_is_collide(PBOX1)         (PBOX1)->is_collide


struct boxcollider2d_t {

    bool is_collide;
    quadf_t *__vertices;
};

boxcollider2d_t boxcollider2d_init(quadf_t *vertices)
{
    return (boxcollider2d_t ) {
        .is_collide = false,
        .__vertices = vertices,
    };
}

void __impl_boxcollider2d_collision_using_AABB(boxcollider2d_t *a, boxcollider2d_t *b)
{
    // AABB - AABB collision
    //
    quadf_t *left = a->__vertices;
    quadf_t *right = b->__vertices;
    
    bool x_axis_collision = 
        (left->vertex[1].cmp[X] >= right->vertex[0].cmp[X]) 
        && (right->vertex[1].cmp[X] >= left->vertex[0].cmp[X]);

    bool y_axis_collision = 
        (left->vertex[0].cmp[Y] >= right->vertex[3].cmp[Y]) 
        && (right->vertex[0].cmp[Y] >= left->vertex[3].cmp[Y]);

    a->is_collide = b->is_collide =  (x_axis_collision && y_axis_collision);
}



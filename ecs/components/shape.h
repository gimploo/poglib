#pragma once
#include "../../math/la.h"
#include "../../simple/gl/types.h"
#include "../components/transform.h"






//TODO: Outline, thickness

typedef struct c_shape2d_t c_shape2d_t ;



#define         c_shape2d_init(POS, TYPE, RADIUS, FILL)       __impl_c_shape2d_init((POS), (CST_ ## TYPE), (RADIUS), (FILL))





#ifndef IGNORE_C_SHAPE_IMPLEMENTATION

#define MAX_TOTAL_CIRCLE_SIDES 10

typedef enum c_shape_type {

    CST_TRIANGLE = 0,
    CST_SQUARE,
    CST_CIRCLE,

    CST_COUNT

} c_shape_type;

struct c_shape2d_t {

    vec3f_t         center_pos;
    c_shape_type    type;
    f32             radius;
    vec4f_t         fill;
    u32             vertex_count;

    void (*update)(c_shape2d_t *shape);
    void (*rotate)(c_shape2d_t *shape, f32 speed, f32 radians);

    // buffer to hold all the vertices
    u8 __vertices[KB];


};


void __c_shape2d_rotate(c_shape2d_t *shape, f32 speed, f32 radians)
{
    vec2f_t *vertices = (vec2f_t *)shape->__vertices;

    for (u32 i = 0; i < shape->vertex_count; i++)
    {
        vec2f_t *tmp = &vertices[i];
        *tmp = (vec2f_t ){
            .cmp[X] = (f32)cos(radians) * tmp->cmp[X] - (f32)sin(radians)*tmp->cmp[Y],
            .cmp[Y] = (f32)sin(radians) * tmp->cmp[X] + (f32)cos(radians)*tmp->cmp[Y],
        };
    }

}

void __c_shape2d_update(c_shape2d_t *cs)
{
    assert(cs);

    vec3f_t centerpos   = cs->center_pos;
    c_shape_type type   = cs->type;
    f32 radius          = cs->radius;
    u8 *__vertices      = cs->__vertices;
    vec2f_t pos2d       = { centerpos.cmp[X] - radius/2, centerpos.cmp[Y] + radius/2  };

    switch(type)
    {
        //Circle
        case CST_CIRCLE: {
            
            circle_t circle = circle_init(*(vec2f_t *)&centerpos, radius);
            memcpy(__vertices, circle.points, sizeof(circle.points));
            cs->vertex_count = MAX_VERTICES_PER_CIRCLE;

        } break;

        //Triangle
        case CST_TRIANGLE:
        {
            trif_t tri = trif_init(pos2d, radius);
            memcpy(__vertices, &tri, sizeof(tri));
            cs->vertex_count = 3;
        }
        break;

        // Square
        case CST_SQUARE:
        {
            quadf_t tmp = quadf_init(pos2d, radius, radius);
            memcpy(__vertices, &tmp, sizeof(tmp));
            cs->vertex_count = 4;
        }
        break;
        default: eprint("side unaccounted for");
    }
}


c_shape2d_t * __impl_c_shape2d_init(vec3f_t pos, c_shape_type type, f32 radius, vec4f_t fill)
{

    c_shape2d_t *o = (c_shape2d_t *)calloc(1, sizeof(c_shape2d_t));
    *o = (c_shape2d_t ){
        .center_pos = pos,
        .type = type,
        .radius = radius,
        .fill = fill,
        .update = __c_shape2d_update,
        .rotate = __c_shape2d_rotate
    };

    return o;
}

#endif 

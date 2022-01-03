#pragma once
#include "../../math/la.h"
#include "../../simple/gl/types.h"
#include "../components/transform.h"






//TODO: Outline, thickness

typedef struct c_shape2d_t c_shape2d_t ;



#define         c_shape2d_init(PCTRANSFORM, TYPE, RADIUS, FILL)     __impl_c_shape2d_init((PCTRANSFORM), (CST_ ## TYPE), (RADIUS), (FILL))
#define         c_shape2d_update(PCS2D)                             (PCS2D)->update(PCS2D)







#ifndef IGNORE_C_SHAPE_IMPLEMENTATION

#define MAX_TOTAL_CIRCLE_SIDES 10

typedef enum c_shape_type {

    CST_CIRCLE      = 0,
    CST_TRIANGLE    = 3,
    CST_SQUARE      = 4,

    CST_COUNT

} c_shape_type;

struct c_shape2d_t {

    vec3f_t         *position;
    c_shape_type    type;
    f32             radius;
    vec3f_t         fill;

    void (*update)(c_shape2d_t *);

    // buffer to hold all the vertices
    u8 __vertices[KB];


};

void __c_shape2d_update(c_shape2d_t *cs)
{
    assert(cs);

    vec3f_t pos         = *cs->position;
    c_shape_type type   = cs->type;
    f32 radius          = cs->radius;
    u8 *__vertices      = cs->__vertices;
    vec2f_t pos2d       = { pos.cmp[X], pos.cmp[Y] };

    switch(type)
    {
        //Circle
        case CST_CIRCLE:
        {
            u64 sides = MAX_TOTAL_CIRCLE_SIDES;

            u8 buffer[sizeof(vec3f_t ) * 10] = {0};

            for(u64 ii = 0; ii < sides; ii++)
            {
                f32 theta = 2.0f * 3.1415926f * (float)ii / (float)sides;//get the current angle

                f32 x = radius * cosf(theta);//calculate the x component
                f32 y = radius * sinf(theta);//calculate the y component

                vec3f_t tmp = { x + pos.cmp[X],  y + pos.cmp[Y], 0.0f };

                memcpy(buffer + ii * sizeof(vec3f_t ), &tmp, sizeof(vec3f_t ));
            }
            memcpy(__vertices, buffer, sizeof(buffer));
        }
        break;

        //Triangle
        case CST_TRIANGLE:
        {
            trif_t tri = trif_init(pos2d, (f32)radius);
            memcpy(__vertices, &tri, sizeof(tri));
        }
        break;

        // Square
        case CST_SQUARE:
        {
            quadf_t tmp = quadf_init(pos2d, radius, radius);
            memcpy(__vertices, &tmp, sizeof(tmp));
        }
        break;
        default: eprint("side unaccounted for");
    }
}


c_shape2d_t * __impl_c_shape2d_init(c_transform_t *ct, c_shape_type type, f32 radius, vec3f_t fill)
{
    assert(ct);

    c_shape2d_t *o = (c_shape2d_t *)calloc(1, sizeof(c_shape2d_t));
    *o = (c_shape2d_t ){
        .position = &ct->position,
        .type = type,
        .radius = radius,
        .fill = fill,
        .update = __c_shape2d_update
    };

    __c_shape2d_update(o);

    return o;
}

#endif 

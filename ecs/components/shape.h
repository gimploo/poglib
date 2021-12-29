#pragma once
#include "../../math/la.h"
#include "../../simple/gl/types.h"






//TODO: Outline, thickness

typedef struct c_shape_t c_shape_t ;



#define         c_shape_init(VEC3F, TYPE, RADIUS, FILL)   __impl_c_shape_init((VEC3F), CST_ ## TYPE, (RADIUS), (FILL))








#ifndef IGNORE_C_SHAPE_IMPLEMENTATION


typedef enum c_shape_type {

    CST_CIRCLE      = 0,
    CST_TRIANGLE    = 3,
    CST_SQUARE      = 4,

    CST_COUNT

} c_shape_type;

struct c_shape_t {

    c_shape_type type;
    vec3f_t position;
    f32 radius;
    vec4f_t fill;

    // buffer to hold all the vertices
    u8 __vertices[KB];

};

void __setup_vertices(vec3f_t pos, u8 __vertices[], c_shape_type type, f32 radius)
{
    vec2f_t pos2d = { pos.cmp[X], pos.cmp[Y] };
    switch(type)
    {
        //Circle
        case CST_CIRCLE:
        {
            u32 sides = 10;
            u8 buffer[sizeof(vec3f_t) * 10] = {0};
            for(int ii = 0; ii < sides; ii++)
            {
                f32 theta = 2.0f * 3.1415926f * float(ii) / float(sides);//get the current angle

                f32 x = radius * cosf(theta);//calculate the x component
                f32 y = radius * sinf(theta);//calculate the y component

                vec3f_t tmp = { x + pos.cmp[X],  y + pos.cmp[Y], 0.0f };

                memcpy(buffer + ii * sizeof(vec3f_t ), &tmp, sizeof(vec3f_t ));
            }
            memcpy(__vertices, buffer, KB);
        }
        break;

        //Triangle
        case CST_TRIANGLE:
        {
            trif_t tri = trif_init(pos2d, (f32)radius);
            memcpy(__vertices, &tri, sizeof(trif_t ));
        }
        break;

        // Square
        case CST_SQUARE:
        {
            quadf_t tmp = quadf_init(pos2d, radius, radius);
            memcpy(__vertices, &tmp, sizeof(quadf_t ));
        }
        break;
        default: eprint("side unaccounted for");
    }
}

c_shape_t __impl_c_shape_init(vec3f_t position, c_shape_type type, f32 radius, vec4f_t fill)
{
    c_shape_t o =  {
        .type = type,
        .radius = radius,
        .fill = fill,
    };

    __setup_vertices(position, o.__vertices, type, radius);

    return o;
}

#endif 

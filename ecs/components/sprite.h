#pragma once
#include "../../simple/gl/types.h"
#include "../../simple/gl/shader.h"
#include "../../simple/gl/texture2d.h"
#include "../components/shape.h"



typedef struct c_sprite_t c_sprite_t ;


c_sprite_t c_sprite_init(c_shape2d_t shape, vec2f_t uv[4], vec3f_t color[4]);


#ifndef IGNORE_C_SPRITE_IMPLEMENTATION

struct c_sprite_t {

    quadf_t uv;
    vec3f_t color[4];
};

 
c_sprite_t c_sprite_init(c_shape2d_t shape, vec3f_t color[4], quadf_t uv)
{
    c_sprite_t o = {0};

    switch(shape.type)
    {
        case CST_TRIANGLE: {
            for (int i = 0; i < 3; i++) 
            {
                o.uv[i].texture_coord = uv.vertex[i];
                o.color = color;
            }
        } break;

        case CST_CIRCLE: {
            for (u64 i = 0; i < MAX_TOTAL_CIRCLE_SIDES; i++)
            {
                uv.vertex[i].cmp[X] = (uv.vertex[i].cmp[X]/shape.radius + 1)*0.5;
                uv.vertex[i].cmp[Y] = (uv.vertex[i].cmp[Y]/shape.radius + 1)*0.5;

                o.vertices[i].color = color[i];
            }
        } break;

        case CST_SQUARE: {
            for (int i = 0; i < 4; i++) 
            {
                o.vertices[i].texture_coord = uv.vertex[i];
                o.vertices[i].color = color[i];
            }
        } break;

        default: eprint("shape type not accounted");
    }

    return o;
}


#endif

#pragma once
#include "../../simple/gl/types.h"
#include "../../simple/gl/shader.h"
#include "../../simple/gl/texture2d.h"
#include "../components/shape.h"



typedef struct c_sprite_t c_sprite_t ;


c_sprite_t c_sprite_init(c_shape2d_t shape, quadf_t uv);


#ifndef IGNORE_C_SPRITE_IMPLEMENTATION

struct c_sprite_t {

    quadf_t uv;
};

 
c_sprite_t c_sprite_init(c_shape2d_t shape, quadf_t uv)
{
    c_sprite_t o = {0};

    switch(shape.type)
    {
        case CST_TRIANGLE: {
            o.uv = uv;
        } break;

        case CST_CIRCLE: {
            for (u64 i = 0; i < MAX_TOTAL_CIRCLE_SIDES; i++)
            {
                uv.vertex[i].cmp[X] = (uv.vertex[i].cmp[X]/shape.radius + 1)*0.5;
                uv.vertex[i].cmp[Y] = (uv.vertex[i].cmp[Y]/shape.radius + 1)*0.5;
            }
        } break;

        case CST_SQUARE: {
            o.uv = uv;
        } break;

        default: eprint("shape type not accounted");
    }

    return o;
}


#endif

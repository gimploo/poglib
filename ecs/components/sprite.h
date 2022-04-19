#pragma once
#include "../../simple/gl/types.h"
#include "../../simple/gl/shader.h"
#include "../../simple/gl/texture2d.h"
#include "../components/shape.h"



typedef struct c_sprite_t c_sprite_t ;


c_sprite_t c_sprite(f32 side, c_shape_type type, quadf_t uv);


#ifndef IGNORE_C_SPRITE_IMPLEMENTATION

struct c_sprite_t {

    quadf_t uv;
};

 
c_sprite_t c_sprite(f32 side, c_shape_type type, quadf_t uv)
{
    c_sprite_t o = {0};

    switch(type)
    {
        case TRIANGLE: {
            o.uv = uv;
        } break;

        case CIRCLE: {
            for (u64 i = 0; i < MAX_TRIANGLES_PER_CIRCLE; i++)
            {
                uv.vertex[i].cmp[X] = (uv.vertex[i].cmp[X]/side + 1)*0.5;
                uv.vertex[i].cmp[Y] = (uv.vertex[i].cmp[Y]/side + 1)*0.5;
            }
        } break;

        case SQUARE: {
            o.uv = uv;
        } break;

        default: eprint("shape type not accounted");
    }

    return o;
}


#endif

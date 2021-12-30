#pragma once
#include "../../simple/gl/types.h"



typedef struct c_sprite_t c_sprite_t ;




#ifndef IGNORE_C_SPRITE_IMPLEMENTATION

struct c_sprite_t {

    glquad_t vertexbuffer;

};


c_sprite_t c_sprite_init(quadf_t pos, vec3f_t color, quadf_t uv, u8 texid)
{
    return (c_sprite_t ) {
        .vertexbuffer = glquad_init(pos, color, uv, texid)
    };
}


#endif

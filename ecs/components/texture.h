#pragma once
#include "../../simple/gl/texture2d.h"
#include "../components/shader.h"



typedef struct c_texture2d_t c_texture2d_t ;

c_texture2d_t   c_texture2d_init(const char *file);
void            c_texture2d_destroy(c_texture2d_t *tx);




#ifndef IGNORE_C_TEXTURE_IMPLEMENTATION

struct c_texture2d_t {

    gltexture2d_t gltexture2d;

};

c_texture2d_t c_texture2d_init(const char *file)
{
    assert(file);

    return (c_texture2d_t ) {

        .gltexture2d = gltexture2d_init(file)

    };
}

void c_texture2d_destroy(c_texture2d_t *tx)
{
    assert(tx);
    gltexture2d_destroy(&tx->gltexture2d);
}

#endif

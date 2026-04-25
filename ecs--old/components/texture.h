#pragma once
#include <poglib/gfx/gl/texture2d.h>
#include "../components/shader.h"


typedef struct c_texture2d_t {

    const char *label;
    const gltexture2d_t *gltexture2d;

}c_texture2d_t ;

c_texture2d_t   c_texture2d(const assetmanager_t *, const char *);


#ifndef IGNORE_C_TEXTURE_IMPLEMENTATION


c_texture2d_t c_texture2d(const assetmanager_t *manager, const char *label)
{
    gltexture2d_t *texture = assetmanager_get_texture2d(manager, label);

    return (c_texture2d_t ) {
        .label = label,
        .gltexture2d = texture

    };
}


#endif

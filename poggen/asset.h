#pragma once
#include "../str.h"
#include "../simple/gl/texture2d.h"
#include "../simple/gl/shader.h"

typedef enum asset_type {

    AT_GLSHADER,

    AT_GLTEXTURE2D,
    AT_GLSPRITE,

    AT_SOUND_WAV,

    AT_FONT_BITMAP,
    AT_FONT_TTF,

    AT_COUNT

} asset_type;

typedef struct asset_t asset_t ;


#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION

struct asset_t {

    str_t       label;
    const char  *filepath;
    asset_type  type;

    union {

        gltexture2d_t   texture2d;
        glshader_t      shader;

    };
};

#endif

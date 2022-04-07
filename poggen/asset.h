#pragma once
#include "../str.h"
#include "../simple/gl/texture2d.h"
#include "../simple/gl/shader.h"
#include "../font/glfreetypefont.h"

typedef enum asset_type {

    AT_GLSHADER,

    AT_GLTEXTURE2D,

    AT_SOUND_WAV,

    AT_FONT_FreeType,

    AT_COUNT

} asset_type;

typedef struct asset_t asset_t ;


#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION

struct asset_t {

    const char  *label;
    const char  *filepath01;
    const char  *filepath02;
    asset_type  type;

    union {

        gltexture2d_t   texture2d;
        glshader_t      shader;
        glfreetypefont_t font;

    };
};

#endif

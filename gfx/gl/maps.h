#pragma once
#include "common.h"

typedef struct gllightmap_t {
    const char *label;
    GLuint diffuse;
    GLuint specular;
    f32 shininess;
} gllightmap_t ;

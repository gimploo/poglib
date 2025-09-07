#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>

#define MAX_TEXTURES_MATERIAL_HOLDS 20

typedef struct glmaterial_t {

    const char *label;
    vec3f_t     ambient;
    vec3f_t     diffuse;
    vec3f_t     specular;
    f32         shininess;
    struct {
        u8 count;
        u8 textureIds[MAX_TEXTURES_MATERIAL_HOLDS];
    } textures;

} glmaterial_t ;



#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>

typedef struct glmaterial_t {

    const char *label;
    vec3f_t     ambient;
    vec3f_t     diffuse;
    vec3f_t     specular;
    f32         shininess;

} glmaterial_t ;



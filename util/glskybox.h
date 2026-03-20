#pragma once
#include "poglib/gfx/gl/types.h"
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>

typedef enum SKYBOX_TYPE {
    SKYBOX_TYPE_BLOOD = 0,
    SKYBOX_TYPE_COUNT
} SKYBOX_TYPE;

const str_t AVAILABLE_SKYBOXES[SKYBOX_TYPE_COUNT][TOTAL_CUBE_FACES] = {
    [SKYBOX_TYPE_BLOOD] = {
        [FRONT]     = str("res/skybox/blood/front.png"),
        [BACK]      = str("res/skybox/blood/back.png"),
        [TOP]       = str("res/skybox/blood/top.png"),
        [BOTTOM]    = str("res/skybox/blood/bottom.png"),
        [LEFT]      = str("res/skybox/blood/left.png"),
        [RIGHT]     = str("res/skybox/blood/right.png")
    }
};

typedef struct {
    const SKYBOX_TYPE type;
    const gltexture2d_t textures[TOTAL_CUBE_FACES];
} glskybox_t;

glskybox_t  glskybox_init(const SKYBOX_TYPE type);
void        glskybox_destroy(glskybox_t *self);

#ifndef IGNORE_GL_SKYBOX_IMPLEMENTATION

glskybox_t glskybox_init(const SKYBOX_TYPE type)
{
    return (glskybox_t) {
        .type = type,
        .textures = {
            [FRONT] = 
        }
    }
}

void glskybox_destroy(glskybox_t *self)
{
    gltexture2d_destroy(&self->texture);
}

#endif

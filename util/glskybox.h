#pragma once
#include "poglib/basic/common.h"
#include "poglib/external/cglm/struct/affine.h"
#include "poglib/gfx/gl/cubemap.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/math/la.h"
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>

typedef enum SKYBOX_TYPE {
    SKYBOX_TYPE_BLOOD = 0,
    SKYBOX_TYPE_COUNT
} SKYBOX_TYPE;

const str_t AVAILABLE_SKYBOXES[SKYBOX_TYPE_COUNT][TOTAL_CUBE_FACES] = {
    [SKYBOX_TYPE_BLOOD] = {
        [FRONT]     = str_lit(POGLIB_ROOT_DIR "/res/skybox/blood/front.png"),
        [BACK]      = str_lit(POGLIB_ROOT_DIR "/res/skybox/blood/back.png"),
        [TOP]       = str_lit(POGLIB_ROOT_DIR "/res/skybox/blood/top.png"),
        [BOTTOM]    = str_lit(POGLIB_ROOT_DIR "/res/skybox/blood/bottom.png"),
        [LEFT]      = str_lit(POGLIB_ROOT_DIR "/res/skybox/blood/left.png"),
        [RIGHT]     = str_lit(POGLIB_ROOT_DIR "/res/skybox/blood/right.png")
    }
};

typedef struct {
    SKYBOX_TYPE type;
    glcubemap_t cubemap;
    glshader_t shader;
} glskybox_t;

glskybox_t      glskybox_init(const SKYBOX_TYPE type, arena_t * const arena);
glrendercall_t  glskybox_get_render_config(const glskybox_t *self, const matrix4f_t projection, const matrix4f_t view);
void            glskybox_destroy(glskybox_t *self);

#ifndef IGNORE_GL_SKYBOX_IMPLEMENTATION

glskybox_t glskybox_init(const SKYBOX_TYPE type, arena_t * const arena)
{
    return (glskybox_t) {
        .type = type,
        .cubemap = glcubemap_init(AVAILABLE_SKYBOXES[type]),
        .shader = glshader__file_init(
            str(POGLIB_ROOT_DIR "/util/glskybox/glskybox-shader-vs.glsl"),
            str(POGLIB_ROOT_DIR "/util/glskybox/glskybox-shader-fg.glsl"),
            arena
        )
    };
}

void glskybox_destroy(glskybox_t *self)
{
    glshader_destroy((glshader_t *)&self->shader);
    glcubemap_destroy((glcubemap_t *)&self->cubemap);
}

glrendercall_t glskybox_get_render_config(
        const glskybox_t *self, 
        const matrix4f_t projection,
        const matrix4f_t view)
{
    return (glrendercall_t) {
        .draw_mode = GL_TRIANGLES,
        .vtx = (buffer_t ){
            .raw_data = (u8 *)DEFAULT_CUBE_VERTICES_24,
            .size = sizeof(DEFAULT_CUBE_VERTICES_24),
        },
        .idx = {
            .data = (u8 *)DEFAULT_CUBE_INDICES_24,
            .nmemb = ARRAY_LEN(DEFAULT_CUBE_INDICES_24)
        },
        .attrs = {
            .count = 1,
            .attr = {
                [0] = {
                    .type = GL_FLOAT,
                    .ncmp = 3,
                    .interleaved = {0}
                }
            }
        },
        .textures = {
            .count = 1,
            .items = {
                [0] = {
                    .type = GL_TEXTURE_TYPE_CUBEMAP,
                    .source.cubemap = (glcubemap_t *)&self->cubemap 
                },
            }
        }, 
        .shader_config = {
            .shader = &self->shader,
            .uniforms = {
                .count = 3,
                .uniform = {
                    [0] = {
                        .name = "projection",
                        .type = "matrix4f_t",
                        .value.mat4 = projection
                    },
                    [1] = {
                        .name = "view",
                        .type = "matrix4f_t",
                        .value.mat4 = view
                    },
                    [2] = {
                        .name = "transform",
                        .type = "matrix4f_t",
                        .value.mat4 = glms_scale_make(vec3f(50.0f))
                    }
                }
            }
        },
    };
}

#endif

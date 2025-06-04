#pragma once
#include <poglib/poggen.h>
#include "./glcamera.h"
#include <poglib/gfx/glrenderer2d.h>
#include <poglib/gfx/glrenderer3d.h>

typedef struct {

    glshader_t shader;
    glcamera_t world_camera;
    vec3f_t player_camera_position;

} workbench_t;

#define WORKBENCH_CAMERA_DEFAULT_POSITION (vec3f_t){-125.f, 40.0f, 200.0f}
#define WORKBENCH_CAMERA_DEFAULT_ROTATION (vec2f_t){-0.3f, -0.9f}

workbench_t workbench_init(const application_t *app)
{
    const char *base_dir = app->context.base_dir;
    char *vshader = mem_init(NULL, 255);
    char *fshader = mem_init(NULL, 255);
    cstr_combine_path(base_dir, "lib/poglib/util/workbench/workbench-shader.vs", vshader, 255);
    cstr_combine_path(base_dir, "lib/poglib/util/workbench/workbench-shader.fs", fshader, 255);

    return (workbench_t) {
        .shader = glshader_from_file_init(
            vshader, 
            fshader),
        .world_camera = glcamera_perspective(
            WORKBENCH_CAMERA_DEFAULT_POSITION,
            WORKBENCH_CAMERA_DEFAULT_ROTATION 
        ),
        .player_camera_position = vec3f(0.f)
    };
}

void workbench_update_player_camera_position(workbench_t *self, const vec3f_t pos)
{
    self->player_camera_position = pos;
}

void workbench_render(workbench_t *self)
{
    glrenderer3d_draw((glrendererconfig_t){
        .calls = {
            .count = 2,
            .call = {
                //camera
                [0] = {
                    .textures = {0},
                    .idx = {
                        .data = (u8 *)&DEFAULT_CUBE_INDICES_8,
                        .nmemb = ARRAY_LEN(DEFAULT_CUBE_INDICES_8)
                    },
                    .vtx = {
                        .data = (u8 *)&DEFAULT_CUBE_VERTICES_8,
                        .size = sizeof(DEFAULT_CUBE_VERTICES_8)
                    },
                    .textures = {0},
                    .attrs = {
                        .count = 1,
                        .attr = {
                            [0] = {
                                .ncmp = 3, 
                                .interleaved = {0}
                            }
                        },
                    },
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .uniform = {
                                [0] = {
                                    .name = "view",
                                    .type = "matrix4f_t",
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value = glms_perspective(
                                        radians(45), global_poggen->handle.app->window.aspect_ratio, 1.0f, 1000.0f)
                                },
                                [2] = {
                                    .name = "color",
                                    .type = "vec4f_t",
                                    .value.vec4 = COLOR_RED
                                },
                                [3] = {
                                    .name = "transform",
                                    .type = "matrix4f_t",
                                    .value = glms_mat4_mul(
                                        glms_translate_make(self->player_camera_position),
                                        glms_scale_make((vec3f_t){2.f, 2.f, 2.f})
                                    ),
                                },
                            }
                        },
                    }
                },
                //platform
                [1] = {
                    .textures = {0},
                    .idx = {
                        .data = (u8 *)&DEFAULT_CUBE_INDICES_8,
                        .nmemb = ARRAY_LEN(DEFAULT_CUBE_INDICES_8)
                    },
                    .vtx = {
                        .data = (u8 *)&DEFAULT_CUBE_VERTICES_8,
                        .size = sizeof(DEFAULT_CUBE_VERTICES_8)
                    },
                    .textures = {0},
                    .attrs = {
                        .count = 1,
                        .attr = {
                            [0] = {
                                .ncmp = 3, 
                                .interleaved = {0}
                            }
                        },
                    },
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .uniform = {
                                [0] = {
                                    .name = "view",
                                    .type = "matrix4f_t",
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value = glms_perspective(
                                        radians(45), 
                                        global_poggen->handle.app->window.aspect_ratio, 
                                        1.0f, 1000.0f
                                    )
                                },
                                [2] = {
                                    .name = "transform",
                                    .type = "matrix4f_t",
                                    .value = glms_scale(MATRIX4F_IDENTITY, (vec3f_t){1000.0f, 1.0f, 1000.0f}),
                                },
                                [3] = {
                                    .name = "color",
                                    .type = "vec4f_t",
                                    .value.vec4 = COLOR_DARK_GRAY
                                }
                            }
                        } 
                    }
                }
            },
        },
    });
}

void workbench_destroy(workbench_t *self)
{
    free((void *)self->shader.vs_file_path);
    free((void *)self->shader.fg_file_path);
    glshader_destroy(&self->shader);
}




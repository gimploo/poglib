#pragma once
#include "poglib/basic/color.h"
#include "poglib/gfx/gl/common.h"
#include <poglib/gfx/glrenderer3d.h>

const f32 GRID_VERTICES_3D[] = {
    // Lines parallel to Z-axis (Verticals in XZ space)
    -10.0f, 0.0f, -10.0f, -10.0f, 0.0f,  10.0f,
    -9.0f,  0.0f, -10.0f, -9.0f,  0.0f,  10.0f,
    -8.0f,  0.0f, -10.0f, -8.0f,  0.0f,  10.0f,
    -7.0f,  0.0f, -10.0f, -7.0f,  0.0f,  10.0f,
    -6.0f,  0.0f, -10.0f, -6.0f,  0.0f,  10.0f,
    -5.0f,  0.0f, -10.0f, -5.0f,  0.0f,  10.0f,
    -4.0f,  0.0f, -10.0f, -4.0f,  0.0f,  10.0f,
    -3.0f,  0.0f, -10.0f, -3.0f,  0.0f,  10.0f,
    -2.0f,  0.0f, -10.0f, -2.0f,  0.0f,  10.0f,
    -1.0f,  0.0f, -10.0f, -1.0f,  0.0f,  10.0f,
    0.0f,  0.0f, -10.0f,  0.0f,  0.0f,  10.0f,
    1.0f,  0.0f, -10.0f,  1.0f,  0.0f,  10.0f,
    2.0f,  0.0f, -10.0f,  2.0f,  0.0f,  10.0f,
    3.0f,  0.0f, -10.0f,  3.0f,  0.0f,  10.0f,
    4.0f,  0.0f, -10.0f,  4.0f,  0.0f,  10.0f,
    5.0f,  0.0f, -10.0f,  5.0f,  0.0f,  10.0f,
    6.0f,  0.0f, -10.0f,  6.0f,  0.0f,  10.0f,
    7.0f,  0.0f, -10.0f,  7.0f,  0.0f,  10.0f,
    8.0f,  0.0f, -10.0f,  8.0f,  0.0f,  10.0f,
    9.0f,  0.0f, -10.0f,  9.0f,  0.0f,  10.0f,
    10.0f,  0.0f, -10.0f, 10.0f,  0.0f,  10.0f,

    // Lines parallel to X-axis (Horizontals in XZ space)
    -10.0f, 0.0f, -10.0f,  10.0f, 0.0f, -10.0f,
    -10.0f, 0.0f,  -9.0f,  10.0f, 0.0f,  -9.0f,
    -10.0f, 0.0f,  -8.0f,  10.0f, 0.0f,  -8.0f,
    -10.0f, 0.0f,  -7.0f,  10.0f, 0.0f,  -7.0f,
    -10.0f, 0.0f,  -6.0f,  10.0f, 0.0f,  -6.0f,
    -10.0f, 0.0f,  -5.0f,  10.0f, 0.0f,  -5.0f,
    -10.0f, 0.0f,  -4.0f,  10.0f, 0.0f,  -4.0f,
    -10.0f, 0.0f,  -3.0f,  10.0f, 0.0f,  -3.0f,
    -10.0f, 0.0f,  -2.0f,  10.0f, 0.0f,  -2.0f,
    -10.0f, 0.0f,  -1.0f,  10.0f, 0.0f,  -1.0f,
    -10.0f, 0.0f,   0.0f,  10.0f, 0.0f,   0.0f,
    -10.0f, 0.0f,   1.0f,  10.0f, 0.0f,   1.0f,
    -10.0f, 0.0f,   2.0f,  10.0f, 0.0f,   2.0f,
    -10.0f, 0.0f,   3.0f,  10.0f, 0.0f,   3.0f,
    -10.0f, 0.0f,   4.0f,  10.0f, 0.0f,   4.0f,
    -10.0f, 0.0f,   5.0f,  10.0f, 0.0f,   5.0f,
    -10.0f, 0.0f,   6.0f,  10.0f, 0.0f,   6.0f,
    -10.0f, 0.0f,   7.0f,  10.0f, 0.0f,   7.0f,
    -10.0f, 0.0f,   8.0f,  10.0f, 0.0f,   8.0f,
    -10.0f, 0.0f,   9.0f,  10.0f, 0.0f,   9.0f,
    -10.0f, 0.0f,  10.0f,  10.0f, 0.0f,  10.0f
};


void workbench_render_grid(
    const glshader_t *shader,
    const matrix4f_t camera_view,
    const matrix4f_t perspective_projection,
    const vec3f_t camera_pos
){
    glrenderer3d_draw((glrendererconfig_t) { .calls = { .count = 1, .call = {
        [0] = {
            .draw_mode = GL_LINES,
            .vtx = {
                .size = sizeof(GRID_VERTICES_3D),
                .data = (u8 *)GRID_VERTICES_3D,
            },
            .shader_config = {
                .shader = shader,
                .uniforms = {
                    .count = 5,
                    .uniform = {
                        [0] = {
                            .name = "view",
                            .type = "matrix4f_t",
                            .value = camera_view
                        },
                        [1] = {
                            .name = "projection",
                            .type = "matrix4f_t",
                            .value = perspective_projection,
                        },
                        [2] = {
                            .name = "transform",
                            .type = "matrix4f_t",
                            .value = MATRIX4F_IDENTITY
                        },
                        [3] = {
                            .name = "color",
                            .type = "vec4f_t",
                            .value.vec4 = COLOR_NOT_AS_BRIGHT_AS_WHITE
                        },
                        [4] = {
                            .name = "cameraPos",
                            .type = "vec3f_t",
                            .value.vec3 = camera_pos
                        },
                    }
                }
            },
            .attrs = {
                .count = 1,
                .attr = {
                    [0] = {
                        .ncmp = 3,
                        .interleaved = 0,
                        .type = GL_FLOAT
                    }
                }
            }
        }
    }}});
}


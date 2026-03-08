#pragma once
#include "poglib/gfx/gl/common.h"
#include <poglib/gfx/glrenderer3d.h>

const f32 GRID_VERTICES_3D[] = {
    // --- Lines parallel to X-axis ---
    -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  0.0f,  1.0f, -1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,
    -1.0f,  0.0f, -1.0f,  1.0f,  0.0f, -1.0f,
    -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  1.0f,  1.0f,  0.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,

    // --- Lines parallel to Y-axis ---
    -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  0.0f, -1.0f,  1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,
     0.0f, -1.0f, -1.0f,  0.0f,  1.0f, -1.0f,
     0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.0f, -1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,
     1.0f, -1.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,

    // --- Lines parallel to Z-axis ---
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  0.0f, -1.0f, -1.0f,  0.0f,  1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
     0.0f, -1.0f, -1.0f,  0.0f, -1.0f,  1.0f,
     0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.0f,  1.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,
     1.0f,  0.0f, -1.0f,  1.0f,  0.0f,  1.0f,
     1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f
};


void workbench_render_grid(
    const glshader_t *shader,
    const matrix4f_t camera_view,
    const matrix4f_t perspective_projection
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
                    .count = 4,
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
                            .value.vec4 = COLOR_BLUE
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


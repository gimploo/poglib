#pragma once
#include <poglib/poggen.h>
#include <poglib/external/joltc/include/joltc.h>

#define DR_MAX_LINES 131072
#define DR_MAX_TRIS  65536

typedef struct {
    JPH_DebugRenderer *jolt_handle;

    f32 *line_vertex_data;
    u32 *line_color_data;
    u32 line_count;
    u32 line_capacity;

    f32 *tri_vertex_data;
    u32 *tri_color_data;
    u32 tri_count;
    u32 tri_capacity;

    bool initialized;
} workbench_debug_renderer_t;

static vec4f_t dr_jph_color_to_vec4f(JPH_Color c)
{
    f32 r = (f32)((c >> 24) & 0xFF) / 255.0f;
    f32 g = (f32)((c >> 16) & 0xFF) / 255.0f;
    f32 b = (f32)((c >> 8) & 0xFF) / 255.0f;
    f32 a = (f32)(c & 0xFF) / 255.0f;
    return (vec4f_t){r, g, b, a};
}

static void JPH_API_CALL dr_draw_line(void *user_data, const JPH_RVec3 *from, const JPH_RVec3 *to, JPH_Color color)
{
    workbench_debug_renderer_t *dr = (workbench_debug_renderer_t *)user_data;
    if (!dr || dr->line_count >= dr->line_capacity) return;

    u32 idx = dr->line_count;
    dr->line_vertex_data[idx * 6 + 0] = from->x;
    dr->line_vertex_data[idx * 6 + 1] = from->y;
    dr->line_vertex_data[idx * 6 + 2] = from->z;
    dr->line_vertex_data[idx * 6 + 3] = to->x;
    dr->line_vertex_data[idx * 6 + 4] = to->y;
    dr->line_vertex_data[idx * 6 + 5] = to->z;
    dr->line_color_data[idx] = color;
    dr->line_count++;
}

static void JPH_API_CALL dr_draw_triangle(void *user_data, const JPH_RVec3 *v1, const JPH_RVec3 *v2, const JPH_RVec3 *v3, JPH_Color color, JPH_DebugRenderer_CastShadow cast_shadow)
{
    (void)cast_shadow;
    workbench_debug_renderer_t *dr = (workbench_debug_renderer_t *)user_data;
    if (!dr || dr->tri_count >= dr->tri_capacity) return;

    u32 idx = dr->tri_count;
    dr->tri_vertex_data[idx * 9 + 0] = v1->x;
    dr->tri_vertex_data[idx * 9 + 1] = v1->y;
    dr->tri_vertex_data[idx * 9 + 2] = v1->z;
    dr->tri_vertex_data[idx * 9 + 3] = v2->x;
    dr->tri_vertex_data[idx * 9 + 4] = v2->y;
    dr->tri_vertex_data[idx * 9 + 5] = v2->z;
    dr->tri_vertex_data[idx * 9 + 6] = v3->x;
    dr->tri_vertex_data[idx * 9 + 7] = v3->y;
    dr->tri_vertex_data[idx * 9 + 8] = v3->z;
    dr->tri_color_data[idx] = color;
    dr->tri_count++;
}

static void JPH_API_CALL dr_draw_text_3d(void *user_data, const JPH_RVec3 *position, const char *str, JPH_Color color, float height)
{
    (void)user_data; (void)position; (void)str; (void)color; (void)height;
}

static const JPH_DebugRenderer_Procs dr_procs = {
    .DrawLine = dr_draw_line,
    .DrawTriangle = dr_draw_triangle,
    .DrawText3D = dr_draw_text_3d,
};

void workbench_debug_renderer_init(workbench_debug_renderer_t *dr, arena_t *arena)
{
    dr->line_capacity = DR_MAX_LINES;
    dr->line_vertex_data = arena_reserve(arena, DR_MAX_LINES * 6 * sizeof(f32));
    dr->line_color_data = arena_reserve(arena, DR_MAX_LINES * sizeof(u32));
    dr->line_count = 0;

    dr->tri_capacity = DR_MAX_TRIS;
    dr->tri_vertex_data = arena_reserve(arena, DR_MAX_TRIS * 9 * sizeof(f32));
    dr->tri_color_data = arena_reserve(arena, DR_MAX_TRIS * sizeof(u32));
    dr->tri_count = 0;

    JPH_DebugRenderer_SetProcs(&dr_procs);
    dr->jolt_handle = JPH_DebugRenderer_Create(dr);

    dr->initialized = true;
}

void workbench_debug_renderer_flush(workbench_debug_renderer_t *dr, const glshader_t *shader, const matrix4f_t view, const matrix4f_t proj, const vec3f_t camera_pos)
{
    if (!dr->initialized || (!dr->line_count && !dr->tri_count)) {
        dr->line_count = 0;
        dr->tri_count = 0;
        return;
    }

    u32 i = 0;
    while (i < dr->line_count)
    {
        JPH_Color color = dr->line_color_data[i];
        u32 start = i;
        while (i < dr->line_count && dr->line_color_data[i] == color) i++;
        u32 vertex_count = (i - start) * 2;

        glrenderer3d_drawcall((glrendercall_t){
            .draw_mode = GL_LINES,
            .vtx = {
                [VBO_STREAM_TYPE_GEOMETRY] = {
                    .size = vertex_count * 3 * sizeof(f32),
                    .raw_data = (u8 *)(dr->line_vertex_data + start * 6),
                }
            },
            .shader_config = {
                .shader = shader,
                .uniforms = {
                    .count = 5,
                    .data = {
                        [0] = { .name = str_lit("view"), .value = view },
                        [1] = { .name = str_lit("projection"), .value = proj },
                        [2] = { .name = str_lit("transform"), .value = MATRIX4F_IDENTITY },
                        [3] = { .name = str_lit("color"), .value.vec4 = dr_jph_color_to_vec4f(color) },
                        [4] = { .name = str_lit("cameraPos"), .value.vec3 = camera_pos },
                    }
                }
            },
            .attrs = {
                .count = 1,
                .attr = {
                    [0] = { .ncmp = 3, .interleaved = 0, .type = GL_FLOAT }
                }
            }
        });
    }

    i = 0;
    while (i < dr->tri_count)
    {
        JPH_Color color = dr->tri_color_data[i];
        u32 start = i;
        while (i < dr->tri_count && dr->tri_color_data[i] == color) i++;
        u32 vertex_count = (i - start) * 3;

        glrenderer3d_drawcall((glrendercall_t){
            .draw_mode = GL_TRIANGLES,
            .vtx = {
                [VBO_STREAM_TYPE_GEOMETRY] = {
                    .size = vertex_count * 3 * sizeof(f32),
                    .raw_data = (u8 *)(dr->tri_vertex_data + start * 9),
                }
            },
            .shader_config = {
                .shader = shader,
                .uniforms = {
                    .count = 5,
                    .data = {
                        [0] = { .name = str_lit("view"), .value = view },
                        [1] = { .name = str_lit("projection"), .value = proj },
                        [2] = { .name = str_lit("transform"), .value = MATRIX4F_IDENTITY },
                        [3] = { .name = str_lit("color"), .value.vec4 = dr_jph_color_to_vec4f(color) },
                        [4] = { .name = str_lit("cameraPos"), .value.vec3 = camera_pos },
                    }
                }
            },
            .attrs = {
                .count = 1,
                .attr = {
                    [0] = { .ncmp = 3, .interleaved = 0, .type = GL_FLOAT }
                }
            }
        });
    }

    dr->line_count = 0;
    dr->tri_count = 0;
}

void workbench_debug_renderer_destroy(workbench_debug_renderer_t *dr)
{
    if (dr->jolt_handle) {
        JPH_DebugRenderer_Destroy(dr->jolt_handle);
        dr->jolt_handle = NULL;
    }
    dr->initialized = false;
}

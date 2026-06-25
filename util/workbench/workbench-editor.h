#pragma once
#include "./common.h"
#include "poglib/ecs.h"
#include "poglib/gui.h"
#include "poglib/gfx/glrenderer3d.h"
#include <poglib/math.h>

#define GIZMO_CIRCLE_SEGMENTS 32
#define GIZMO_AXIS_LENGTH 0.15f
#define GIZMO_CONE_HEIGHT 0.25f
#define GIZMO_CONE_RADIUS 0.10f
#define GIZMO_CUBE_SIZE 0.18f
#define GIZMO_RING_RADIUS 0.85f

const vec4f_t GIZMO_COLOR_X = {1.0f, 0.2f, 0.2f, 1.0f};
const vec4f_t GIZMO_COLOR_Y = {0.2f, 1.0f, 0.2f, 1.0f};
const vec4f_t GIZMO_COLOR_Z = {0.2f, 0.4f, 1.0f, 1.0f};
const vec4f_t GIZMO_COLOR_WHITE = {1.0f, 1.0f, 1.0f, 0.8f};


f32 workbench_editor__internal_closest_point_on_ray(const vec3f_t ray_origin, const vec3f_t ray_dir, const vec3f_t targetpoint)
{
    const vec3f_t to_point = glms_vec3_sub(targetpoint, ray_origin);
    f32 t = glms_vec3_dot(to_point, ray_dir);
    if (t < 0.f) t = 0.f;
    vec3f_t closest = glms_vec3_add(ray_origin, glms_vec3_scale(ray_dir, t));
    return glms_vec3_distance(closest, targetpoint);
}

void workbench_editor__internal_check_mouse_closest_entity(void)
{
    vec2f_t ndc = window_mouse_get_norm_position(global_window);
    glcamera_t *cam = global_workbench->world_camera.handle;

    vec3f_t dir = {0};
    {
        const matrix4f_t view       = glcamera_getview(cam);
        const matrix4f_t proj       = glms_perspective(radians(45), global_engine->handle.app->window.aspect_ratio, 0.1f, 1000.0f);
        const matrix4f_t inv_pv     = glms_mat4_inv(glms_mat4_mul(proj, view));
        const vec4f_t near          = { ndc.x, ndc.y, -1.0f, 1.0f };
        const vec4f_t far           = { ndc.x, ndc.y,  1.0f, 1.0f };

        vec4f_t near_w      = glms_mat4_mulv(inv_pv, near);
        vec4f_t far_w       = glms_mat4_mulv(inv_pv, far);
        near_w              = glms_vec4_scale(near_w, 1.0f / near_w.w);
        far_w               = glms_vec4_scale(far_w,  1.0f / far_w.w);

        dir                 = glms_vec3_normalize(glms_vec3_sub(*(vec3f_t *)&far_w, *(vec3f_t *)&near_w));
    }

    u32 picked = 0;
    f32 closest_dist = 5.f;

    slot_iterator(&global_ecs->managers.entitymanager.entities, iter)
    {
        if (!slot_iterator_index) continue;

        const ecs_entity_t *const e         = iter;
        const ecs_entity_query_t q          = ecs_entity_query_components(global_ecs, e->id, ECS_CMP_TRANSFORM);
        const ecs_component_transform_t *t  = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        if (!t) continue;

        const f32 d = workbench_editor__internal_closest_point_on_ray(cam->position, dir, t->position);
        if (d < closest_dist) {
            closest_dist = d;
            picked = e->id;
        }
    }

    global_workbench->editor.mouse_closest_to_entity_id = picked;
}

void workbench_editor__internal_show_entity_info_for_selected_entity(void)
{
    if (!global_workbench->editor.selected_entity_id) return;

    gui_t *const gui = &global_workbench->gui.handle;

    gui_ui_compose_begin(gui, (ui_config_t) {
            .dim = {
                .min_width = 150,
                .min_height = 150,
            },
            .margin = {
                .top = 20.f,
                .left = 20.f
            },
            .color = {
                .base = COLOR_BLACK
            }
    });
    for (u8 idx = 0; idx < 3; idx ++)
    {
        const str_t options[3] = {
            str("T"),
            str("R"),
            str("S"),
        };
            gui_ui_compose_begin(gui, (ui_config_t) {
                .id = idx + 1,
                .composition = {
                    .traits = UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_CLICKABLE 
                },
                .dim = {
                    .min_width = 20,
                    .min_height = 15,
                },
                .margin = {
                    .top = 20.f,
                    .left = 20.f
                },
                .color = {
                    .base = COLOR_GRAY,
                    .highlight = COLOR_DARK_GRAY
                }
            });

                gui_ui_compose_begin(gui, (ui_config_t) {
                        .id = idx + 1,
                        .text = options[idx],
                        .composition = {
                            .styles = UI_STYLE_ONLY_TEXT,
                        },
                        .dim = {
                            .min_width = 10,
                            .min_height = 10,
                        },
                        .color = {
                            .base = COLOR_WHITE
                        }
                }); gui_ui_compose_end(gui);

            gui_ui_compose_end(gui);
    }
    gui_ui_compose_end(gui);
}


static void gizmo__draw_line(vec3f_t a, vec3f_t b, vec4f_t color,
    matrix4f_t view, matrix4f_t proj, vec3f_t cam_pos, glshader_t *shader)
{
    f32 vtx[6] = { a.x, a.y, a.z, b.x, b.y, b.z };
    glrenderer3d_drawcall((glrendercall_t){
        .draw_mode = GL_LINES,
        .vtx = {
            [VBO_STREAM_TYPE_GEOMETRY] = {
                .raw_data = (u8 *)vtx,
                .size = sizeof(vtx)
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
                    [3] = { .name = str_lit("color"), .value.vec4 = color },
                    [4] = { .name = str_lit("cameraPos"), .value.vec3 = cam_pos },
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

static void gizmo__draw_cone(vec3f_t tip, vec3f_t dir, f32 height, f32 radius,
    vec4f_t color, matrix4f_t view, matrix4f_t proj, vec3f_t cam_pos, glshader_t *shader)
{
    vec3f_t base_center = glms_vec3_sub(tip, glms_vec3_scale(dir, height));
    vec3f_t up = {0.0f, 1.0f, 0.0f};
    if (fabs(glms_vec3_dot(dir, up)) > 0.99f)
        up = (vec3f_t){1.0f, 0.0f, 0.0f};
    vec3f_t right = glms_vec3_normalize(glms_vec3_cross(dir, up));
    up = glms_vec3_normalize(glms_vec3_cross(right, dir));

    f32 vtx[8 * 6 * 3];
    u32 count = 0;
    for (u32 i = 0; i < 8; i++) {
        f32 a0 = (f32)i / 8.0f * 2.0f * PI;
        f32 a1 = (f32)(i + 1) / 8.0f * 2.0f * PI;
        vec3f_t p0 = glms_vec3_add(base_center,
            glms_vec3_add(glms_vec3_scale(right, cosf(a0) * radius),
                          glms_vec3_scale(up, sinf(a0) * radius)));
        vec3f_t p1 = glms_vec3_add(base_center,
            glms_vec3_add(glms_vec3_scale(right, cosf(a1) * radius),
                          glms_vec3_scale(up, sinf(a1) * radius)));
        vtx[count++] = tip.x; vtx[count++] = tip.y; vtx[count++] = tip.z;
        vtx[count++] = p0.x;  vtx[count++] = p0.y;  vtx[count++] = p0.z;
        vtx[count++] = p1.x;  vtx[count++] = p1.y;  vtx[count++] = p1.z;
        vtx[count++] = base_center.x; vtx[count++] = base_center.y; vtx[count++] = base_center.z;
        vtx[count++] = p1.x;          vtx[count++] = p1.y;          vtx[count++] = p1.z;
        vtx[count++] = p0.x;          vtx[count++] = p0.y;          vtx[count++] = p0.z;
    }

    glrenderer3d_drawcall((glrendercall_t){
        .draw_mode = GL_TRIANGLES,
        .vtx = {
            [VBO_STREAM_TYPE_GEOMETRY] = {
                .raw_data = (u8 *)vtx,
                .size = count * sizeof(f32)
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
                    [3] = { .name = str_lit("color"), .value.vec4 = color },
                    [4] = { .name = str_lit("cameraPos"), .value.vec3 = cam_pos },
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

static void gizmo__draw_cube(vec3f_t center, f32 size, vec4f_t color, matrix4f_t view, matrix4f_t proj, vec3f_t cam_pos, glshader_t *shader)
{
    const f32 h = size * 0.5f;
    const vec3f_t c = center;
    const vec3f_t corners[8] = {
        {c.x - h, c.y - h, c.z - h},
        {c.x + h, c.y - h, c.z - h},
        {c.x + h, c.y + h, c.z - h},
        {c.x - h, c.y + h, c.z - h},
        {c.x - h, c.y - h, c.z + h},
        {c.x + h, c.y - h, c.z + h},
        {c.x + h, c.y + h, c.z + h},
        {c.x - h, c.y + h, c.z + h},
    };
    const u32 edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7},
    };
    f32 vtx[12 * 2 * 3];
    u32 count = 0;
    for (u32 i = 0; i < 12; i++) {
        vtx[count++] = corners[edges[i][0]].x;
        vtx[count++] = corners[edges[i][0]].y;
        vtx[count++] = corners[edges[i][0]].z;
        vtx[count++] = corners[edges[i][1]].x;
        vtx[count++] = corners[edges[i][1]].y;
        vtx[count++] = corners[edges[i][1]].z;
    }

    glrenderer3d_drawcall((glrendercall_t){
        .draw_mode = GL_LINES,
        .vtx = {
            [VBO_STREAM_TYPE_GEOMETRY] = {
                .raw_data = (u8 *)vtx,
                .size = count * sizeof(f32)
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
                    [3] = { .name = str_lit("color"), .value.vec4 = color },
                    [4] = { .name = str_lit("cameraPos"), .value.vec3 = cam_pos },
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

static void gizmo__draw_ring(vec3f_t center, vec3f_t axis, f32 radius, u32 segments,
    vec4f_t color, matrix4f_t view, matrix4f_t proj, vec3f_t cam_pos, glshader_t *shader)
{
    (void)cam_pos;
    vec3f_t up = {0.0f, 1.0f, 0.0f};
    if (fabs(glms_vec3_dot(axis, up)) > 0.99f)
        up = (vec3f_t){1.0f, 0.0f, 0.0f};
    vec3f_t right = glms_vec3_normalize(glms_vec3_cross(axis, up));
    up = glms_vec3_normalize(glms_vec3_cross(right, axis));

    f32 vtx[GIZMO_CIRCLE_SEGMENTS * 2 * 3];
    u32 count = 0;
    for (u32 i = 0; i < segments; i++) {
        f32 a0 = (f32)i / segments * 2.0f * PI;
        f32 a1 = (f32)(i + 1) / segments * 2.0f * PI;
        vec3f_t p0 = glms_vec3_add(center,
            glms_vec3_add(glms_vec3_scale(right, cosf(a0) * radius),
                          glms_vec3_scale(up, sinf(a0) * radius)));
        vec3f_t p1 = glms_vec3_add(center,
            glms_vec3_add(glms_vec3_scale(right, cosf(a1) * radius),
                          glms_vec3_scale(up, sinf(a1) * radius)));
        vtx[count++] = p0.x; vtx[count++] = p0.y; vtx[count++] = p0.z;
        vtx[count++] = p1.x; vtx[count++] = p1.y; vtx[count++] = p1.z;
    }

    glrenderer3d_drawcall((glrendercall_t){
        .draw_mode = GL_LINES,
        .vtx = {
            [VBO_STREAM_TYPE_GEOMETRY] = {
                .raw_data = (u8 *)vtx,
                .size = count * sizeof(f32)
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
                    [3] = { .name = str_lit("color"), .value.vec4 = color },
                    [4] = { .name = str_lit("cameraPos"), .value.vec3 = cam_pos },
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

void workbench_editor__gizmo_draw(vec3f_t entity_pos, vec3f_t cam_pos, matrix4f_t view, matrix4f_t proj, gizmo_mode_t mode)
{
    f32 dist = glms_vec3_distance(cam_pos, entity_pos);
    f32 scale = dist * GIZMO_AXIS_LENGTH;

    const vec3f_t axes[3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    const vec4f_t colors[3] = {
        GIZMO_COLOR_X, GIZMO_COLOR_Y, GIZMO_COLOR_Z
    };

    glshader_t *shader = &global_workbench->shader;

    for (u32 i = 0; i < 3; i++) {
        vec3f_t tip = glms_vec3_add(entity_pos, glms_vec3_scale(axes[i], scale));

        if (mode == GIZMO_MODE_TRANSLATE) {
            gizmo__draw_line(entity_pos, tip, colors[i], view, proj, cam_pos, shader);
            gizmo__draw_cone(tip, axes[i], scale * GIZMO_CONE_HEIGHT, scale * GIZMO_CONE_RADIUS,
                            colors[i], view, proj, cam_pos, shader);
        } else if (mode == GIZMO_MODE_SCALE) {
            gizmo__draw_line(entity_pos, tip, colors[i], view, proj, cam_pos, shader);
            gizmo__draw_cube(tip, scale * GIZMO_CUBE_SIZE, colors[i], view, proj, cam_pos, shader);
        } else if (mode == GIZMO_MODE_ROTATE) {
            gizmo__draw_ring(entity_pos, axes[i], scale * GIZMO_RING_RADIUS, GIZMO_CIRCLE_SEGMENTS,
                            colors[i], view, proj, cam_pos, shader);
        }
    }

    if (mode == GIZMO_MODE_SCALE) {
        gizmo__draw_cube(entity_pos, scale * GIZMO_CUBE_SIZE * 0.5f,
                        GIZMO_COLOR_WHITE, view, proj, cam_pos, shader);
    }
}

void workbench_editor_render(void)
{
    workbench_editor__internal_show_entity_info_for_selected_entity();


    if (global_workbench->editor.selected_entity_id) {

        ecs_entity_query_t q = ecs_entity_query_components(
            global_ecs, global_workbench->editor.selected_entity_id, ECS_CMP_TRANSFORM
        );

        ecs_component_transform_t *transform = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

        if (transform) {

            const glcamera_t *cam = global_workbench->world_camera.handle;
            const matrix4f_t view = glcamera_getview(cam);
            const matrix4f_t proj = glms_perspective(radians(45),
                global_engine->handle.app->window.aspect_ratio, 1.0f, 1000.0f);


            workbench_editor__gizmo_draw(
                transform->position, cam->position, view, proj,
                global_workbench->editor.gizmo_mode
            );
        }
    }
}

void workbench_editor_update(void)
{
    workbench_editor__internal_check_mouse_closest_entity();
}


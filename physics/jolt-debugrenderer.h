#pragma once
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/util/workbench/common.h"
#include <poglib/external/joltc/include/joltc.h>
#include <poglib/poggen.h>
#include <poglib/ecs.h>

typedef struct {
    poggen_t            *engine;
    glshader_t          *lineshader;
    JPH_DebugRenderer   *handle;
    JPH_DrawSettings    settings;
    struct {
        matrix4f_t view;
        matrix4f_t projection;
    } frame;
} jph_debugrenderer_t;


INTERNAL vec4s jph_color_to_vec4s(const JPH_Color c)
{
    const f32 r = (f32)((c >> 24) & 0xFF) / 255.0f;
    const f32 g = (f32)((c >> 16) & 0xFF) / 255.0f;
    const f32 b = (f32)((c >> 8) & 0xFF) / 255.0f;
    const f32 a = (f32)(c & 0xFF) / 255.0f;
    return (vec4s){r, g, b, a};
}

INTERNAL void joltdebugrenderer_draw__internal__triangle_callback(void *user_data, const JPH_RVec3 *v1, const JPH_RVec3 *v2, const JPH_RVec3 *v3, const JPH_Color color, JPH_DebugRenderer_CastShadow cast_shadow)
{
    jph_debugrenderer_t *const ctx = user_data;
    const mat4s view = glcamera_getview(ecs_get_active_camera(global_ecs));
    const matrix4f_t proj = glms_perspective(
        radians(45),
        ctx->engine->handle.app->window.aspect_ratio,
        1.0f,
        10000.0f
    );

    renderqueue_pass_command(
        &ctx->engine->systems.renderqueue,
        (rendercommand_t) {
            .enable_wireframe = true,
            .vtx = {
                .type = RENDERCOMMAND_VTX_TYPE_TRIANGLES,
                .data.triangles = (rendercommand_primitive_triangle_t){
                    .color = jph_color_to_vec4s(color),
                    .points = {
                        *(vec3s *)v1, *(vec3s *)v2, *(vec3s *)v3,
                    }
                },
            },
            .material = {
                .shader = {
                    .data = ctx->lineshader,
                    .uniforms = {
                        .count = 2,
                        .data = {
                            [0] = { .name = str_lit("view"),        .value = ctx->frame.view },
                            [1] = { .name = str_lit("projection"),  .value = ctx->frame.projection },
                        }
                    }
                }
            }
        }
    );
}

INTERNAL void joltdebugrenderer_draw__internal__line_callback(void *const user_data, const JPH_RVec3 *const from, const JPH_RVec3 *const to, const JPH_Color color)
{
    jph_debugrenderer_t *const ctx = user_data;
    renderqueue_pass_command(
        &ctx->engine->systems.renderqueue,
        (rendercommand_t) {
            .vtx = {
                .type = RENDERCOMMAND_VTX_TYPE_LINE,
                .data.line = {
                    .color  = jph_color_to_vec4s(color),
                    .start  = *(vec3s *)from,
                    .end    = *(vec3s *)to
                },
            },
            .material = {
                .shader = {
                    .data = ctx->lineshader,
                    .uniforms = {
                        .count = 2,
                        .data = {
                            [0] = { .name = str_lit("view"),        .value = ctx->frame.view },
                            [1] = { .name = str_lit("projection"),  .value = ctx->frame.projection },
                        }
                    }
                }
            }
        }
    );
}

INTERNAL void joltdebugrenderer_draw__internal__text_callback(void *user_data, const JPH_RVec3 *position, const char *str, JPH_Color color, float height)
{
    eprint("not implemented");
}

const JPH_DebugRenderer_Procs joltdebugrenderer_callbacks = {
    .DrawLine       = joltdebugrenderer_draw__internal__line_callback,
    .DrawTriangle   = joltdebugrenderer_draw__internal__triangle_callback,
    .DrawText3D     = joltdebugrenderer_draw__internal__text_callback,
};

jph_debugrenderer_t * joltdebugrenderer_init(void)
{
    ASSERT(global_workbench);
    ASSERT(global_engine);

    const glshader_t *const line_shader = (glshader_t *)assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, global_workbench->primitives.line_shader_id);
    ASSERT(line_shader);

    jph_debugrenderer_t *jph_debugrenderer = mem_init(&(jph_debugrenderer_t){
        .engine         = global_engine,
        .lineshader     = line_shader,
    }, sizeof(*jph_debugrenderer));

    JPH_DebugRenderer_SetProcs(&joltdebugrenderer_callbacks);
    jph_debugrenderer->handle = JPH_DebugRenderer_Create(jph_debugrenderer);

    jph_debugrenderer->settings = (JPH_DrawSettings){0};
    JPH_DrawSettings_InitDefault(&jph_debugrenderer->settings);
    jph_debugrenderer->settings.drawShape            = true;
    jph_debugrenderer->settings.drawShapeWireframe   = false;
    jph_debugrenderer->settings.drawShapeColor       = JPH_BodyManager_ShapeColor_InstanceColor;

    return jph_debugrenderer;
}

void joltdebugrenderer_render(jph_debugrenderer_t *const self)
{
    if (!global_workbench->enable_collider) return;

    self->frame.view = glcamera_getview(ecs_get_active_camera(global_ecs));
    self->frame.projection = glms_perspective(
        radians(45),
        self->engine->handle.app->window.aspect_ratio,
        1.0f,
        10000.0f
    );

    JPH_DebugRenderer_NextFrame(self->handle);
    JPH_PhysicsSystem_DrawBodies(
        global_physics_sys_jolt_instance->physics_system,
        &self->settings,
        self->handle,
        NULL
    );
}

void joltdebugrenderer_destroy(jph_debugrenderer_t *const self)
{
    ASSERT(global_workbench);
    JPH_DebugRenderer_Destroy(self->handle);
    mem_free(self, sizeof(*self));
}


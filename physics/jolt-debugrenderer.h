#pragma once
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/util/glcamera.h"
#include <poglib/external/joltc/include/joltc.h>
#include <poglib/util/workbench/common.h>

INTERNAL vec4s jph_color_to_vec4s(const JPH_Color c)
{
    const f32 r = (f32)((c >> 24) & 0xFF) / 255.0f;
    const f32 g = (f32)((c >> 16) & 0xFF) / 255.0f;
    const f32 b = (f32)((c >> 8) & 0xFF) / 255.0f;
    const f32 a = (f32)(c & 0xFF) / 255.0f;
    return (vec4s){ r, g, b, a};
}

INTERNAL void joltdebugrenderer_draw__internal__triangle_callback(void *user_data, const JPH_RVec3 *v1, const JPH_RVec3 *v2, const JPH_RVec3 *v3, const JPH_Color color, const JPH_DebugRenderer_CastShadow cast_shadow)
{
    if (global_workbench->disable_joltrenderer) return;

    jolt_debugrenderer_t *const ctx = user_data;

    renderqueue_pass_command(
        ctx->renderqueue,
        (rendercommand_t) {
            .enable_wireframe = true,
            .vtx = {
                .type = RENDERCOMMAND_VTX_TYPE_TRIANGLES,
                .data.triangle = (rendercommand_primitive_triangle_t){
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
    if (global_workbench->disable_joltrenderer) return;

    jolt_debugrenderer_t *const ctx = user_data;
    renderqueue_pass_command(
        ctx->renderqueue,
        (rendercommand_t) {
            .vtx = {
                .type = RENDERCOMMAND_VTX_TYPE_LINE,
                .data.line = {
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
    if (global_workbench->disable_joltrenderer) return;
    eprint("not implemented");
}

const JPH_DebugRenderer_Procs joltdebugrenderer_callbacks = {
    .DrawLine       = joltdebugrenderer_draw__internal__line_callback,
    .DrawTriangle   = joltdebugrenderer_draw__internal__triangle_callback,
    .DrawText3D     = joltdebugrenderer_draw__internal__text_callback,
};

jolt_debugrenderer_t * joltdebugrenderer_init(
    renderqueue_t *const renderqueue, 
    glshader_t *const line_shader
) {
    jolt_debugrenderer_t *const jph_debugrenderer = mem_init(&(jolt_debugrenderer_t){
        .renderqueue    = renderqueue,
        .lineshader     = line_shader,
    }, sizeof(*jph_debugrenderer));

    jph_debugrenderer->handle   = JPH_DebugRenderer_Create(jph_debugrenderer);
    JPH_DebugRenderer_SetProcs(&joltdebugrenderer_callbacks);
    JPH_DrawSettings_InitDefault(&jph_debugrenderer->settings);
    jph_debugrenderer->settings.drawShape            = true;
    jph_debugrenderer->settings.drawShapeWireframe   = false;
    jph_debugrenderer->settings.drawShapeColor       = JPH_BodyManager_ShapeColor_InstanceColor;

    return jph_debugrenderer;
}

void joltdebugrenderer_update(
    jolt_debugrenderer_t *const self,
    const glcamera_t *const camera,
    const f32 aspect_ratio)
{
    JPH_DebugRenderer_NextFrame(self->handle);

    self->frame.view = glcamera_getview(camera);
    self->frame.projection = glms_perspective(
        radians(45),
        aspect_ratio,
        1.0f,
        10000.0f
    );
}

void joltdebugrenderer_render_colliders(
    jolt_debugrenderer_t *const self,
    JPH_PhysicsSystem *const joltphysicssystem
) {
    ASSERT(joltphysicssystem);

    JPH_PhysicsSystem_DrawBodies(
        joltphysicssystem,
        &self->settings,
        self->handle,
        NULL
    );
}

void joltdebugrenderer_destroy(jolt_debugrenderer_t *const self)
{
    JPH_DebugRenderer_Destroy(self->handle);
    mem_free(self, sizeof(*self));
}


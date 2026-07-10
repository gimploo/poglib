#pragma once
#include "poglib/ecs/component/types.h"
#include "poglib/external/cglm/struct/mat4.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/math/la.h"
#include "poglib/util/glcamera.h"
#include <poglib/basic.h>

typedef enum {
    UNIFORM_SOURCE_CAMERA_VIEW,
    UNIFORM_SOURCE_CAMERA_PROJECTION,
    UNIFORM_SOURCE_ENTITY_TRANSFORM,
    UNIFORM_SOURCE_MODEL_COLOR,
    UNIFORM_SOURCE_BONE_TRANSFORMS,
    UNIFORM_SOURCE_LIGHT_COLOR,
    UNIFORM_SOURCE_LIGHT_AMBIENT,
    UNIFORM_SOURCE_LIGHT_POSITION,
    UNIFORM_SOURCE_COUNT
} uniform_source_t;

typedef struct {
    str_t name;
    gluniform_type type;
    uniform_source_t source;
} uniform_binding_t;

typedef struct {
    const glcamera_t *active_camera;
    f32 aspect_ratio;
    const ecs_component_transform_t *transform;
    vec4f_t model_color;
    struct {
        u32 count;
        matrix4f_t *data;
    } bones;
    bool is_selected;
} uniform_compute_ctx_t;

static const uniform_binding_t UNIFORM_REGISTRY[] = {
    { str_lit("view"),           GL_UNIFORM_TYPE_MATRIX4F,       UNIFORM_SOURCE_CAMERA_VIEW        },
    { str_lit("projection"),     GL_UNIFORM_TYPE_MATRIX4F,       UNIFORM_SOURCE_CAMERA_PROJECTION   },
    { str_lit("transform"),      GL_UNIFORM_TYPE_MATRIX4F,       UNIFORM_SOURCE_ENTITY_TRANSFORM    },
    { str_lit("material.color"), GL_UNIFORM_TYPE_VEC4F,          UNIFORM_SOURCE_MODEL_COLOR         },
    { str_lit("uBones"),         GL_UNIFORM_TYPE_MATRIX4F_ARRAY, UNIFORM_SOURCE_BONE_TRANSFORMS     },
    { str_lit("light.color"),    GL_UNIFORM_TYPE_VEC4F,          UNIFORM_SOURCE_LIGHT_COLOR         },
    { str_lit("light.ambient"),  GL_UNIFORM_TYPE_F32,            UNIFORM_SOURCE_LIGHT_AMBIENT       },
    { str_lit("light.position"), GL_UNIFORM_TYPE_VEC3F,          UNIFORM_SOURCE_LIGHT_POSITION      },
};
#define UNIFORM_REGISTRY_COUNT ARRAY_LEN(UNIFORM_REGISTRY)

static const uniform_binding_t *uniform_registry_lookup(const str_t name)
{
    for (u8 i = 0; i < UNIFORM_REGISTRY_COUNT; i++)
        if (str_cmp(UNIFORM_REGISTRY[i].name, name))
            return &UNIFORM_REGISTRY[i];
    return NULL;
}

static gluniform_value_t uniform_registry_compute(uniform_source_t source, const uniform_compute_ctx_t *ctx)
{
    switch (source)
    {
        case UNIFORM_SOURCE_CAMERA_VIEW:
            if (ctx->active_camera) return (gluniform_value_t){ .mat4 = glcamera_getview(ctx->active_camera) };
        break;

        case UNIFORM_SOURCE_CAMERA_PROJECTION:
            return (gluniform_value_t){ .mat4 = glms_perspective(radians(45), ctx->aspect_ratio, 1.0f, 1000.0f) };

        case UNIFORM_SOURCE_ENTITY_TRANSFORM:
            if (ctx->transform)
                return (gluniform_value_t){ .mat4 = glms_mat4_mul(
                    glms_translate_make(ctx->transform->position),
                    glms_mat4_mul(
                        glms_quat_mat4(ctx->transform->orientation),
                        glms_scale_make(ctx->transform->scale)
                    )
                )};
        break;

        case UNIFORM_SOURCE_MODEL_COLOR:
            return (gluniform_value_t){ .vec4 = ctx->model_color };

        case UNIFORM_SOURCE_BONE_TRANSFORMS:
            return (gluniform_value_t){ .mat4s = { .count = ctx->bones.count, .data = ctx->bones.data } };

        case UNIFORM_SOURCE_LIGHT_COLOR:
            return (gluniform_value_t){ .vec4 = ctx->is_selected ? COLOR_RED : COLOR_WHITE };

        case UNIFORM_SOURCE_LIGHT_AMBIENT:
            return (gluniform_value_t){ .f32 = 1.0f };

        case UNIFORM_SOURCE_LIGHT_POSITION:
            return (gluniform_value_t){ .vec3 = vec3f(1.0f, 1.0f, 1.0f) };

        case UNIFORM_SOURCE_COUNT:
        break;
    }
    return (gluniform_value_t){0};
}

static bool uniform_registry_apply_material_overrides(gluniforms_t *cmd_uniforms, const gluniforms_t *material_overrides)
{
    for (u8 m = 0; m < material_overrides->count; m++)
    {
        const uniform_binding_t *binding = uniform_registry_lookup(material_overrides->data[m].name);
        if (!binding) continue;

        for (u8 c = 0; c < cmd_uniforms->count; c++)
        {
            if (str_cmp(cmd_uniforms->data[c].name, material_overrides->data[m].name)) {
                cmd_uniforms->data[c].value = material_overrides->data[m].value;
                goto next_override;
            }
        }
        next_override:;
    }
    return true;
}

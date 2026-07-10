#pragma once
#include "poglib/ecs/component/types.h"
#include "poglib/gfx/gl/shader.h"
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

INTERNAL const uniform_binding_t *uniform_registry_lookup(const str_t name)
{
    for (u8 i = 0; i < UNIFORM_REGISTRY_COUNT; i++)
        if (str_cmp(UNIFORM_REGISTRY[i].name, name))
            return &UNIFORM_REGISTRY[i];
    return NULL;
}

#pragma once
#include "poglib/poggen/input/commandqueue.h"
#include "poglib/util/asset.h"
#include <poglib/basic.h>
#include <poglib/math.h>

typedef enum {

    ECS_CMP_TRANSFORM_IDX   = 0,
    ECS_CMP_MODEL_IDX       = 1,
    ECS_CMP_INPUT_IDX       = 2,
    ECS_CMP_MATERIAL_IDX    = 3,
    ECS_CMP_COUNT

} ecs_component_storage_index;

typedef enum {

    ECS_CMP_TRANSFORM   = 1 << ECS_CMP_TRANSFORM_IDX,
    ECS_CMP_MODEL       = 1 << ECS_CMP_MODEL_IDX,
    ECS_CMP_INPUT       = 1 << ECS_CMP_INPUT_IDX,
    ECS_CMP_MATERIAL    = 1 << ECS_CMP_MATERIAL_IDX,

} ecs_component_type;

typedef struct ecs_component_transform_t ecs_component_transform_t;
struct ecs_component_transform_t {

    vec3f_t translation;
    vec3f_t rotation;
    vec3f_t scale;
    enum {
        ECS_CMP_TRANSFORM_SOURCE_MANUAL,
        ECS_CMP_TRANSFORM_SOURCE_PHYSICS,
        ECS_CMP_TRANSFORM_SOURCE_ANIMATION,
    } source;
    struct {
        u32 entity_id;
    } internal;

};

typedef struct ecs_component_model_t ecs_component_model_t;
struct ecs_component_model_t {

    asset_id asset_id;

};

typedef struct ecs_component_input_t ecs_component_input_t;
struct ecs_component_input_t {

    vec3f_t move_dir;
    vec3f_t speed;
    f32 delta_time;
    void (*callback)(ecs_component_input_t *const self, const u16 command_bitmask);

};

typedef struct ecs_component_material_t ecs_component_material_t;
struct ecs_component_material_t {
    asset_id textureid;
    asset_id shaderid;
};


typedef struct ecs_componentbundle_t ecs_componentbundle_t;
struct ecs_componentbundle_t {

    u32 signature;                                          //NOTE: this holds the bitmask of all compnents configured for the entity
    struct {
        union {
            ecs_component_transform_t   transform;
            ecs_component_model_t       model;
            ecs_component_input_t       input;
            ecs_component_material_t    material;
        };
    } component[ECS_CMP_COUNT];

};

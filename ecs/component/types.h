#pragma once
#include "poglib/ecs/common.h"
#include "poglib/poggen/input/commandqueue.h"
#include "poglib/util/asset.h"
#include <poglib/basic.h>
#include <poglib/math.h>


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

typedef struct ecs_component_mesh_t ecs_component_mesh_t;
struct ecs_component_mesh_t {

    asset_id asset_id;

};

typedef struct ecs_component_input_t ecs_component_input_t;
struct ecs_component_input_t {

    vec3f_t move_dir;
    vec3f_t speed;
    f32 delta_time;
    void (*callback)(ecs_component_input_t *const self, const u16 command_bitmask);

};


typedef struct ecs_componentbundle_t ecs_componentbundle_t;
struct ecs_componentbundle_t {

    u32 signature;                                          //NOTE: this holds the bitmask of all compnents configured for the entity
    struct {
        union {
            ecs_component_transform_t transform;
            ecs_component_mesh_t mesh;
            ecs_component_input_t input;
        }; 
    } component[ECS_CMP_COUNT];

};

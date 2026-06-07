#pragma once
#include "poglib/input/commandqueue.h"
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/physics/jolt-wrapper.h>
#include <poglib/util/glcamera.h>

#define ECS_CMP_INVALID_IDX -1

typedef enum {

    ECS_CMP_TRANSFORM_IDX       = 0,
    ECS_CMP_MODEL_IDX           = 1,
    ECS_CMP_INPUT_IDX           = 2,
    ECS_CMP_MATERIAL_IDX        = 3,
    ECS_CMP_CAMERA_IDX          = 4,
    ECS_CMP_CHARACTER_IDX       = 5,
    ECS_CMP_STATIC_COLLIDER_IDX = 6,
    ECS_CMP_COUNT

} ecs_component_storage_index;

typedef enum {

    ECS_CMP_TRANSFORM           = 1 << ECS_CMP_TRANSFORM_IDX,
    ECS_CMP_MODEL               = 1 << ECS_CMP_MODEL_IDX,
    ECS_CMP_INPUT               = 1 << ECS_CMP_INPUT_IDX,
    ECS_CMP_MATERIAL            = 1 << ECS_CMP_MATERIAL_IDX,
    ECS_CMP_CAMERA              = 1 << ECS_CMP_CAMERA_IDX,
    ECS_CMP_CHARACTER           = 1 << ECS_CMP_CHARACTER_IDX,
    ECS_CMP_STATIC_COLLIDER     = 1 << ECS_CMP_STATIC_COLLIDER_IDX,

} ecs_component_type;

/*========================= TRANSFORM ====================================== */

typedef struct ecs_component_transform_t ecs_component_transform_t;
struct ecs_component_transform_t {

    vec3f_t position;
    versors orientation;
    vec3f_t scale;
    // controls which system writes to this transform: NONE (externally set), MANUAL (from input), PHYSICS (from physics engine)
    enum {
        ECS_CMP_TRANSFORM_SOURCE_NONE,
        ECS_CMP_TRANSFORM_SOURCE_MANUAL,
        ECS_CMP_TRANSFORM_SOURCE_PHYSICS,
        ECS_CMP_TRANSFORM_SOURCE_ANIMATION,
    } source;
};

/*========================= MODEL/MESH ====================================== */

typedef struct ecs_component_model_t ecs_component_model_t;
struct ecs_component_model_t {
    u32 asset_id;
};

/*========================= INPUT ====================================== */

typedef struct ecs_component_input_t        ecs_component_input_t;
typedef struct ecs_component_input_state_t  ecs_component_input_state_t;

struct ecs_component_input_state_t {
    versors     current_orientation;
    vec3f_t     current_position;
    vec3f_t     front;
    vec3f_t     right;
    vec3f_t     desired_velocity;    // set by input_behavior for movement systems to consume
};

struct ecs_component_input_t {
    enum {
        ECS_CMP_INPUT_DIRECTION_SOURCE_ENTITY,
        ECS_CMP_INPUT_DIRECTION_SOURCE_CAMERA,
    } direction_source;
    void (*input_behavior)(ecs_component_input_state_t * const state, const u16 command_bitmask, const f32 dt);
    struct {
        commandqueue_t *commandqueue;
        ecs_component_input_state_t state; 
    } internal;
};

/* =========================== MATERIAL ================================== */

typedef struct ecs_component_material_t ecs_component_material_t;
struct ecs_component_material_t {
    u32 texture_asset_id;
    u32 shader_asset_id;
};

/* =========================== CAMERA ================================== */

typedef enum ecs_component_camera_mode {
    ECS_CMP_CAMERA_MODE_FREE_FLY    = 0,
    ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW      = 1,
} ecs_component_camera_mode;

typedef struct ecs_component_camera_t {
    glcamera_t camera;
    ecs_component_camera_mode mode;
    struct {
        f32 orbit_radius;
        vec3f_t center_offset;
        u32 track_entity_id;
    } follow;
} ecs_component_camera_t;

/* =========================== COLLISION TYPES =============================== */

typedef enum {
    PHYS_COLLIDER_TYPE_CAPSULE = 1,
    PHYS_COLLIDER_TYPE_SPHERE = 2,
    PHYS_COLLIDER_TYPE_CUBE = 3,
    PHYS_COLLIDER_TYPE_COUNT,
} collider_type;

typedef union {
    vec3f_t dim;
    struct {
        f32 height;
        f32 radius;
    };
} collider_dimension_t;

/* =========================== COLLISION ================================== */

typedef struct ecs_component_character_t ecs_component_character_t;
struct ecs_component_character_t {
    JPH_CharacterVirtual *character;           // Jolt character controller (NULL until lazy init)
    u16 object_layer;                          // Jolt object layer for collision filtering
    f32 half_height;                           // capsule half-height
    f32 radius;                                // capsule radius
    f32 stick_to_floor_distance;               // max step-down distance to stay on ground
    f32 walk_stairs_step;                      // max step-up height for stairs
};

/* =========================== STATIC COLLIDER ============================= */

typedef struct ecs_component_static_collider_t ecs_component_static_collider_t;
struct ecs_component_static_collider_t {
    JPH_BodyID body_id;                        // Jolt body ID (0 until lazy init in character-collision system)
    collider_type type;                        // shape type (CUBE, CAPSULE, etc.)
    JPH_ObjectLayer layer;                     // Jolt object layer for collision filtering
    collider_dimension_t dimension;            // shape dimensions (union of vec3f_t dim or {height, radius})
};


/* =========================== MISC ==========================================*/

typedef struct ecs_component_poolentry_t    ecs_component_poolentry_t;
typedef struct ecs_componentbundle_t        ecs_componentbundle_t;
typedef struct ecs_cmp_patch_payload_t      ecs_cmp_patch_payload_t;

struct ecs_component_poolentry_t {
    u32 entity_id;
    bool is_active;
    alignas(16) u8 entity_cmpdata[];
};

#define ECS_CMP_POOL_HEADER_SIZE offsetof(ecs_component_poolentry_t, entity_cmpdata)

struct ecs_componentbundle_t {

    //NOTE: this holds the bitmask of all compnents configured for the entity
    u32 signature;
    struct {
        union {
            ecs_component_transform_t   transform;
            ecs_component_model_t       model;
            ecs_component_input_t       input;
            ecs_component_material_t    material;
            ecs_component_camera_t          camera;
            ecs_component_character_t       character;
            ecs_component_static_collider_t static_collider;
        };
    } component[ECS_CMP_COUNT];

};

struct ecs_cmp_patch_payload_t
{
    enum {
        ECS_PATCH_CMP_ACTIVE_FIELD = 0,
    } patch_type;
    u32 signature;
    bool is_active;
};


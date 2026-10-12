#pragma once
#include "poglib/gfx/gl/framebuffer.h"
#include "poglib/util/asset.h"
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/external/joltc/include/joltc.h>
#include <poglib/util/glcamera.h>
#include <poglib/gfx/gl/shader.h>
#include <poglib/gfx/gl/texture_types.h>
#include <poglib/gfx/model/glmodel.h>

#define ECS_CMP_INVALID_IDX -1

typedef enum {

    ECS_CMP_TRANSFORM_IDX       = 0,
    ECS_CMP_MODEL_IDX           = 1,
    ECS_CMP_INPUT_IDX           = 2,
    ECS_CMP_MATERIAL_IDX        = 3,
    ECS_CMP_CAMERA_IDX          = 4,
    ECS_CMP_COLLIDER_IDX        = 5,
    ECS_CMP_MESH_IDX            = 6,
    ECS_CMP_SPRITE_IDX          = 7,
    ECS_CMP_LIGHT_IDX           = 8, 
    ECS_CMP_ANIMATION_IDX       = 9, 
    ECS_CMP_COUNT

} ecs_component_storage_index;

typedef enum {

    ECS_CMP_TRANSFORM           = 1 << ECS_CMP_TRANSFORM_IDX,
    ECS_CMP_MODEL               = 1 << ECS_CMP_MODEL_IDX,
    ECS_CMP_INPUT               = 1 << ECS_CMP_INPUT_IDX,
    ECS_CMP_MATERIAL            = 1 << ECS_CMP_MATERIAL_IDX,
    ECS_CMP_CAMERA              = 1 << ECS_CMP_CAMERA_IDX,
    ECS_CMP_COLLIDER            = 1 << ECS_CMP_COLLIDER_IDX,
    ECS_CMP_MESH                = 1 << ECS_CMP_MESH_IDX,
    ECS_CMP_SPRITE              = 1 << ECS_CMP_SPRITE_IDX,
    ECS_CMP_LIGHT               = 1 << ECS_CMP_LIGHT_IDX,
    ECS_CMP_ANIMATION           = 1 << ECS_CMP_ANIMATION_IDX,

} ecs_component_type;

/*========================= TRANSFORM ====================================== */

typedef struct ecs_component_transform_t ecs_component_transform_t;
struct ecs_component_transform_t {

    vec3f_t position;
    versors orientation;
    vec3f_t scale;
    vec3f_t velocity;
    enum {
        ECS_CMP_TRANSFORM_SOURCE_NONE,
        ECS_CMP_TRANSFORM_SOURCE_INPUT,
    } source;
};

/*========================= MODEL ====================================== */

typedef struct ecs_component_model_t ecs_component_model_t;
struct ecs_component_model_t {
    u32 asset_id;
    u16 mesh_idx;
    struct {
        glmodel_t *model;
    } internal;
};

/*========================= MESH ====================================== */

#define ECS_MODEL_MAX_MESH_COUNT 4
typedef struct ecs_component_mesh_t ecs_component_mesh_t;
struct ecs_component_mesh_t {
    u32 asset_id;
    u16 mesh_idx;
    u8 primitive_idx;
    bool is_scene_instanced;
};

/*========================= INPUT ====================================== */

typedef struct ecs_component_input_t        ecs_component_input_t;
typedef struct ecs_component_input_state_t  ecs_component_input_state_t;

struct ecs_component_input_state_t {
    versors     current_orientation;
    vec3f_t     current_position;
    vec3f_t     local_velocity;
    vec3f_t     front;
    vec3f_t     right;
    vec3f_t     up;
};

struct ecs_component_input_t {
    enum {
        ECS_CMP_INPUT_DIRECTION_SOURCE_ENTITY,
        ECS_CMP_INPUT_DIRECTION_SOURCE_CAMERA,
    } direction_source;
    void (*input_behavior)(ecs_component_input_state_t * const state, const u16 command_bitmask, const f32 dt);
    struct {
        ecs_component_input_state_t state; 
    } internal;
};

/* =========================== MATERIAL ================================== */

typedef struct ecs_component_material_t ecs_component_material_t;
struct ecs_component_material_t {

    mesh2texture_lookup_t meshidx_to_texture_lookup;
    u32 shader_asset_id;

    struct {
        gluniforms_t uniform_values;
    } internal;
};

/* =========================== SPRITE ================================== */

typedef struct ecs_component_sprite_t ecs_component_sprite_t;
struct ecs_component_sprite_t {
    u32 spritesheet_asset_id;
    u32 sprite_idx; 
};

/* =========================== CAMERA ================================== */

typedef enum ecs_component_camera_mode {
    ECS_CMP_CAMERA_MODE_FREE_FLY            = 0,
    ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW        = 1,
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

/* =========================== COLLIDERS ================================== */

#define ECS_COLLIDER_MAX_MESH_PER_MODEL 3

typedef struct ecs_component_collider_t ecs_component_collider_t;
typedef struct ecs_collider_jolt_userdata_t ecs_collider_jolt_userdata_t;

typedef enum {
    COLLIDER_SHAPE_TYPE_NONE                = 0,
    COLLIDER_SHAPE_TYPE_CAPSULE             = 1,
    COLLIDER_SHAPE_TYPE_SPHERE              = 2,
    COLLIDER_SHAPE_TYPE_CUBE                = 3,
    COLLIDER_SHAPE_TYPE_CYLINDER            = 4,
    COLLIDER_SHAPE_TYPE_MESH                = 5,
    COLLIDER_SHAPE_TYPE_MODEL               = 6,
    COLLIDER_SHAPE_TYPE_COUNT,
} collider_shape_type;

typedef union {
    vec3s raw;
    struct {
        f32 half_width;
        f32 half_height;
        f32 half_depth;
    } cube;
    struct {
        f32 radius;
    } sphere;
    struct {
        f32 radius;
        f32 half_height;
    } capsule;
    struct {
        f32 radius;
        f32 half_height;
    } cylinder;
    struct {
        struct {
            u32 count;
            vec4s *data;
        } vtx;
        struct {
            u32 count;
            u32 *data;
        } idx;
    } mesh;
    struct {
        u8 count;
        cgltf_accessor *vtx_list[ECS_COLLIDER_MAX_MESH_PER_MODEL];
        cgltf_accessor *idx_list[ECS_COLLIDER_MAX_MESH_PER_MODEL];
    } model;
} collider_shape_dimension_t;

typedef struct {
    JPH_BodyID bodyid;
} ecs_component_collider_softbody_t;

typedef struct {
    JPH_BodyID              body_ids[ECS_COLLIDER_MAX_MESH_PER_MODEL];
    JPH_GroundState         groundstate;
    JPH_CharacterVirtual    *kinematicbody;
#ifdef DEBUG
        JPH_Shape           *shapes[ECS_COLLIDER_MAX_MESH_PER_MODEL];
#endif
} ecs_component_collider_rigidbody_t;


struct ecs_component_collider_t {
    enum {
        ECS_COLLIDER_RIGIDBODY = 0,
        ECS_COLLIDER_SOFTBODY = 1,
    } type;
    union {
        ecs_component_collider_rigidbody_t  rigid;
        ecs_component_collider_softbody_t   soft;
    } body;
    collider_shape_type             shape_type;
    JPH_MotionType                  motion_type;
    JPH_ObjectLayer                 object_layer_type;
    collider_shape_dimension_t      dim;
    struct {
        vec3f_t                     position;
        versors                     orientation;
        u32                         entity_id;
        vec3f_t                     scale;
    } internal;
};

struct ecs_collider_jolt_userdata_t {
    JPH_ObjectLayer objectlayertype;
    u32 entity_id;
};

/* =========================== LIGHT ================================== */

typedef struct ecs_component_light_t  ecs_component_light_t;
struct ecs_component_light_t {
    enum {
        POINT_LIGHT = 0,
    } type;
    vec4s color;
    f32 constant;
    f32 linear;
    f32 quadratic;
    f32 radius;
};

/* ========================== ANIMATION ============================ */

typedef struct ecs_component_animation_t ecs_component_animation_t;
struct ecs_component_animation_t {

    u32 asset_id;
    struct {
        glmodel_t *model;
    } internal;
};

/* =========================== MISC ==========================================*/

typedef struct ecs_component_poolentry_t    ecs_component_poolentry_t;  //NOTE: used to iterate over a component pool
typedef struct ecs_componentbundle_t        ecs_componentbundle_t;      //NOTE: used to configure an entity during init
typedef struct ecs_cmp_patch_payload_t      ecs_cmp_patch_payload_t;    //NOTE: used to patch an component held by an entity

struct ecs_component_poolentry_t {
    u32 entity_id;
    bool is_active;
    //NOTE: 11 bytes of empty space use for future usecases
    alignas(16) u8 entity_cmpdata[];
};

#define ECS_CMP_POOL_HEADER_SIZE offsetof(ecs_component_poolentry_t, entity_cmpdata)

struct ecs_componentbundle_t {

    u32 signature; //NOTE: this holds the bitmask of all compnents configured for the entity
    struct {
        union {
            ecs_component_transform_t   transform;
            ecs_component_model_t       model;
            ecs_component_mesh_t        mesh;
            ecs_component_input_t       input;
            ecs_component_material_t    material;
            ecs_component_camera_t      camera;
            ecs_component_collider_t    collider;
            ecs_component_sprite_t      sprite;
            ecs_component_light_t       light;
            ecs_component_animation_t   animation;
        };
    } component[ECS_CMP_COUNT];

};

struct ecs_cmp_patch_payload_t {
    enum {
        ECS_PATCH_CMP_ACTIVE_FIELD = 0,
    } patch_type;
    u32 signature;
    bool is_active;
};


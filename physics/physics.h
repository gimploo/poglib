#pragma once

#include "./jolt-wrapper.h"

//TODO: better to move this into an ECS system.
//


typedef enum {
    PHYS_COLLIDER_TYPE_CAPSULE = 1,
    PHYS_COLLIDER_TYPE_SPHERE = 2,
    PHYS_COLLIDER_TYPE_CUBE = 3,
    PHYS_COLLIDER_TYPE_COUNT,
} collider_type;

typedef struct {
    JPH_BodyID body_id;
    union {
        JPH_ObjectLayer objectlayer;
        collider_type collider_type;
    } collider_type;
} collider_t;

typedef union {

    vec3f_t dim;
    struct {
        f32 height;
        f32 radius;
    };
} collider_dimension_t;


collider_t collider_init(
        const vec3f_t position,
        const collider_dimension_t cd_dim,
        JPH_MotionType jph_motion_type, //TODO: think of way to abstract this away 
        collider_type type,
        bool activate_only_on_impact)
{
    ASSERT(global_physics_sys_jolt_instance);

    JPH_BodyID body_id = 0;
    switch(type) 
    {
        case PHYS_COLLIDER_TYPE_CAPSULE:
            body_id = physics_sys_jolt_create_capsule(
                    global_physics_sys_jolt_instance, 
                    position, 
                    cd_dim.height / 2.0f,
                    cd_dim.radius,
                    jph_motion_type, 
                    type, 
                    !activate_only_on_impact);
        break;
        case PHYS_COLLIDER_TYPE_SPHERE:
            eprint("not implemented");
        break;
        case PHYS_COLLIDER_TYPE_CUBE:
            body_id = physics_sys_jolt_create_box(
                    global_physics_sys_jolt_instance, 
                    position, 
                    (vec3f_t){ cd_dim.dim.x / 2.0f, cd_dim.dim.y / 2.0f, cd_dim.dim.z / 2.0f }, 
                    jph_motion_type, 
                    type, 
                    !activate_only_on_impact);
        break;
        default: eprint("unknown type");
    }

    return (collider_t) {
        .body_id = body_id,
        .collider_type = type
    };
}


void collision_pass_entity_data_to_jolt(JPH_BodyID bodyId, void *entity_data)
{
    JPH_Body_SetUserData(bodyId, entity_data);
}


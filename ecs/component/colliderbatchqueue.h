#pragma once
#include <poglib/physics/jolt-wrapper.h>
#include "./types.h"
#include "poglib/basic/arena.h"
#include "poglib/external/joltc/include/joltc.h"

typedef struct {

    queue_t queue;
    arena_t arena;

} colliderbatchqueue_t;


colliderbatchqueue_t        colliderbatchqueue(arena_t *const arena);
void                        colliderbatchqueue_add(colliderbatchqueue_t *const self, const ecs_component_collider_t *const collider);
void                        colliderbatchqueue_upload_to_jolt(colliderbatchqueue_t *const self);


#ifndef IGNORE_COLLIDER_BATCH_QUEUE_IMPLEMENTATION

colliderbatchqueue_t colliderbatchqueue(arena_t *const arena)
{
    ASSERT(arena);
    colliderbatchqueue_t result = {0};
    result.queue = queue_init(10, ecs_component_collider_t *, arena);
    result.arena = arena_init(arena, KB);
    return result;
}


void colliderbatchqueue_add(colliderbatchqueue_t *const self, const ecs_component_collider_t *const collider)
{
    ASSERT(self);
    queue_put(&self->queue, collider);
}

i32 colliderbatchqueue__internal_qsort_compare(const void *const x, const void *const y)
{
    const ecs_component_collider_t *c1 = *(ecs_component_collider_t **)x;
    const ecs_component_collider_t *c2 = *(ecs_component_collider_t **)y;

    if (c2->motion_type == JPH_MotionType_Static 
        && (c1->motion_type == JPH_MotionType_Dynamic || c1->motion_type == JPH_MotionType_Kinematic)) 
        return 1;

    if (c1->motion_type == JPH_MotionType_Static 
        && (c2->motion_type == JPH_MotionType_Dynamic || c2->motion_type == JPH_MotionType_Kinematic)) 
        return -1;

    if (c1->shape_type > c2->shape_type) return 1;
    if (c1->shape_type < c2->shape_type) return -1;

    if (c1->object_layer_type > c2->object_layer_type) return 1;
    if (c1->object_layer_type < c2->object_layer_type) return -1;

    return memcmp(&c1->dim, &c2->dim, sizeof(collider_shape_dimension_t));
}

void colliderbatchqueue_upload_to_jolt(colliderbatchqueue_t *const self)
{
    if (!self->queue.len) return;

    qsort(self->queue.__data, self->queue.len, sizeof(ecs_component_collider_t *), colliderbatchqueue__internal_qsort_compare);

    JPH_Shape *shape                        = NULL;
    JPH_BodyCreationSettings *body_settings = NULL;
    JPH_CharacterVirtualSettings settings   = {0};

    collider_shape_type prev_shape_type                 = COLLIDER_SHAPE_TYPE_NONE;
    collider_shape_dimension_t prev_shape_dimension     = {0};
    JPH_MotionType prev_motion_type                     = JPH_MotionType_None;

    while(!queue_is_empty(&self->queue))
    {
        ecs_component_collider_t *const collider = queue_get(&self->queue);

        const bool diff_shape_type = prev_shape_type != collider->shape_type;
        const bool diff_shape_dim = memcmp(&prev_shape_dimension, &collider->dim, sizeof(collider_shape_dimension_t)) != 0;
        const bool diff_motion_type = prev_motion_type != collider->motion_type;

        //NOTE: we club all static motion types together before moving on to other motion types, using that behavior we can
        //know when to optimize the broadphase i.e. after all static types are completely done
        bool optimize_broadphase = (diff_motion_type && prev_motion_type == JPH_MotionType_Static) 
            || (!self->queue.len && prev_motion_type == JPH_MotionType_Static);


        if (diff_shape_type || diff_motion_type || diff_shape_dim) {
            JPH_BodyCreationSettings_Destroy(body_settings);
            body_settings = NULL;
        }

        if (diff_shape_type || diff_shape_dim) {

            if (shape) {
#ifndef DEBUG
                JPH_Shape_Destroy(shape); 
#endif
                shape = NULL;
            }

            switch(collider->shape_type)
            {
                case COLLIDER_SHAPE_TYPE_CUBE:
                    shape = (JPH_Shape *)JPH_BoxShape_Create((const JPH_Vec3 *)&collider->dim.cube, JPH_DEFAULT_CONVEX_RADIUS);
                break;
                case COLLIDER_SHAPE_TYPE_CAPSULE:
                    shape = (JPH_Shape *)JPH_CapsuleShape_Create(collider->dim.capsule.half_height, collider->dim.capsule.radius);
                break;
                case COLLIDER_SHAPE_TYPE_SPHERE:
                    shape = (JPH_Shape *)JPH_SphereShape_Create(collider->dim.sphere.radius);
                break;

                default: eprint("Collider shape type not accounted for here");
            }

            collider->internal.shape = shape;
        }

        if(diff_shape_dim || diff_motion_type) {

            switch(collider->motion_type) {
                case JPH_MotionType_Static:
                case JPH_MotionType_Dynamic:
                    body_settings = JPH_BodyCreationSettings_Create3(
                        shape,
                        (JPH_Vec3 *)&collider->internal.position, 
                        (JPH_Quat *)&collider->internal.orientation, 
                        collider->motion_type, 
                        collider->object_layer_type
                    );
                break;
                case JPH_MotionType_Kinematic:
                    JPH_CharacterVirtualSettings_Init(&settings);
                    settings.base.shape = shape;
                    settings.shapeOffset = (JPH_Vec3){ 0, collider->dim.capsule.half_height + collider->dim.capsule.radius, 0 };
                break;
                default: eprint("Unknown motion type");
            }
        }

        switch(collider->motion_type)
        {
            case JPH_MotionType_Static:
            case JPH_MotionType_Dynamic:
                ASSERT(body_settings);
                collider->internal.body_id = JPH_BodyInterface_CreateAndAddBody(
                    global_physics_sys_jolt_instance->bodyinterface, 
                    body_settings, 
                    JPH_Activation_Activate
                );

                //WARN: keeping it here like this so as to keep it easy to make the futher 
                //expansion of `ecs_collider_jolt_userdata_t` whenever we may need to

                const ecs_collider_jolt_userdata_t userdata = {
                    .objectlayertype = collider->object_layer_type,
                    .dimension = collider->dim,
#ifdef DEBUG
                    .internal = {
                        .ecs_collider = collider
                    }
#endif
                };
                JPH_BodyInterface_SetUserData(
                    global_physics_sys_jolt_instance->bodyinterface,
                    collider->internal.body_id,
                    (u64)arena_store(&self->arena, &userdata, sizeof(userdata))
                );
            break;
            case JPH_MotionType_Kinematic:
                collider->internal.kinematic_body = JPH_CharacterVirtual_Create(
                    &settings,
                    (JPH_RVec3 *)&collider->internal.position,
                    (JPH_Quat *) &collider->internal.orientation,
                    0,
                    global_physics_sys_jolt_instance->physics_system
                );
            break;
            default: eprint("motion type not accounted for");
        }

        prev_shape_type         = collider->shape_type;
        prev_shape_dimension    = collider->dim;
        prev_motion_type        = collider->motion_type;

        if (optimize_broadphase) {
            JPH_PhysicsSystem_OptimizeBroadPhase(global_physics_sys_jolt_instance->physics_system);
        }
    }

    if (body_settings)  JPH_BodyCreationSettings_Destroy(body_settings);

#ifndef DEBUG
    if (shape)          JPH_Shape_Destroy(shape); 
#endif

}

#endif

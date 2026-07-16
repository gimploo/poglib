#pragma once
#include <poglib/physics/jolt-wrapper.h>
#include "./types.h"
#include "poglib/basic/arena.h"
#include "poglib/external/joltc/include/joltc.h"

typedef struct {

    queue_t queue;
    arena_t *arena;

} colliderbatchqueue_t;


colliderbatchqueue_t        colliderbatchqueue(arena_t *const arena);
void                        colliderbatchqueue_add(colliderbatchqueue_t *const self, const ecs_component_collider_t *const collider);
void                        colliderbatchqueue_upload_to_jolt(colliderbatchqueue_t *const self);


#ifndef IGNORE_COLLIDER_BATCH_QUEUE_IMPLEMENTATION

colliderbatchqueue_t colliderbatchqueue(arena_t *const arena)
{
    ASSERT(arena);
    colliderbatchqueue_t result = {0};
    result.queue = queue_init(100, ecs_component_collider_t *, arena);
    result.arena = arena_init(arena, 1 * MB);
    return result;
}


void colliderbatchqueue_add(colliderbatchqueue_t *const self, const ecs_component_collider_t *const collider)
{
    ASSERT(self);
    queue_put(&self->queue, collider, sizeof(ecs_component_collider_t *));
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
    JPH_MotionType prev_motion_type         = JPH_MotionType_None;

    while(!queue_is_empty(&self->queue))
    {
        ecs_component_collider_t *const collider    = queue_get(&self->queue);
        const bool diff_motion_type                 = prev_motion_type != collider->motion_type;

        //NOTE: we club all static motion types together before moving on to other motion types, this way we would
        //know when to optimize the broadphase i.e. after all static types are completely done.
        const bool optimize_broadphase = (diff_motion_type && prev_motion_type == JPH_MotionType_Static) 
            || (!self->queue.len && prev_motion_type == JPH_MotionType_Static);

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
            case COLLIDER_SHAPE_TYPE_CYLINDER:
                shape = (JPH_Shape *)JPH_CylinderShape_Create(collider->dim.cylinder.half_height, collider->dim.cylinder.radius);
            break;
            case COLLIDER_SHAPE_TYPE_MESH:
            {
                ASSERT(collider->motion_type == JPH_MotionType_Static);
                const u32 vtx_count = collider->dim.mesh.vtx.count;
                const u32 tri_count = collider->dim.mesh.idx.data 
                    ? collider->dim.mesh.idx.count / 3 
                    : vtx_count / 3;

                vec3f_t *scaled_vtx = arena_reserve(self->arena, vtx_count * sizeof(vec3f_t));
                for (u32 i = 0; i < vtx_count; i++) {
                    scaled_vtx[i].x = collider->dim.mesh.vtx.data[i].x * collider->internal.scale.x;
                    scaled_vtx[i].y = collider->dim.mesh.vtx.data[i].y * collider->internal.scale.y;
                    scaled_vtx[i].z = collider->dim.mesh.vtx.data[i].z * collider->internal.scale.z;
                }

                JPH_IndexedTriangle *const tris = arena_reserve(self->arena, tri_count * sizeof(JPH_IndexedTriangle));
                for (u32 i = 0; i < tri_count; i++) {
                    tris[i] = (JPH_IndexedTriangle){
                        .i1 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 0] : i * 3 + 0,
                        .i2 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 1] : i * 3 + 1,
                        .i3 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 2] : i * 3 + 2,
                    };
                }

                JPH_MeshShapeSettings *settings = JPH_MeshShapeSettings_Create2(
                    (const JPH_Vec3 *)scaled_vtx,
                    vtx_count,
                    tris,
                    tri_count
                );
                JPH_MeshShapeSettings_Sanitize(settings);
                shape = (JPH_Shape *)JPH_MeshShapeSettings_CreateShape(settings);
                JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)settings);

                arena_giveback(self->arena, scaled_vtx, vtx_count * sizeof(vec3f_t));
                arena_giveback(self->arena, tris, tri_count * sizeof(JPH_IndexedTriangle));
            } break;

            default: eprint("Collider shape type not accounted for here");
        }

#ifdef DEBUG
        //NOTE: having this is very confusing, if scales are changed for CUBES during edit - new shape is generated but for 
        //CAPSULE and SPHERE is scaled up so for those alone we only need the original shape
        collider->internal.shape = shape;
#endif

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
                    (u64)arena_store(self->arena, &userdata, sizeof(userdata))
                );
                JPH_BodyCreationSettings_Destroy(body_settings);
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

        if (optimize_broadphase) {
            JPH_PhysicsSystem_OptimizeBroadPhase(global_physics_sys_jolt_instance->physics_system);
            logging("optimizied broadphase\n");
        }

#ifndef DEBUG
        if (shape) {
            JPH_Shape_Destroy(shape);
            shape = NULL;
        }
#endif
        prev_motion_type  = collider->motion_type;
    }

}

#endif

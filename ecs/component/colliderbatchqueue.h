#pragma once
#include "./types.h"
#include "joltc.h"
#include "poglib/basic/arena.h"
#include "poglib/physics/jolt-wrapper.h"

//TODO: have the Editor object layer type be bulk inserted. explore the improvements for those

typedef struct {

    struct {
        buffer_t data;
        i32 top;
    } array_buffer;

    arena_t *arena;

} colliderbatchqueue_t;


colliderbatchqueue_t        colliderbatchqueue(arena_t *const arena);
void                        colliderbatchqueue_add(colliderbatchqueue_t *const self, ecs_component_collider_t *const collider);
void                        colliderbatchqueue_upload_to_jolt(colliderbatchqueue_t *const self);


#ifndef IGNORE_COLLIDER_BATCH_QUEUE_IMPLEMENTATION

colliderbatchqueue_t colliderbatchqueue(arena_t *const arena)
{
    ASSERT(arena);
    const u32 buffersize = sizeof(ecs_component_collider_t *) * 10000;
    return (colliderbatchqueue_t){
        .array_buffer.data = (buffer_t){
            .size       = buffersize,
            .raw_data   = (u8 *)arena_reserve(arena, buffersize)
        },
        .array_buffer.top = -1,
        .arena = arena_init(arena, 1 * MB),
    };
}


void colliderbatchqueue_add(colliderbatchqueue_t *const self, ecs_component_collider_t *const collider)
{
    ASSERT(self);
    ASSERT(collider);

    memcpy(
        (ecs_component_collider_t **)self->array_buffer.data.raw_data + ++self->array_buffer.top, 
        &collider, 
        sizeof(ecs_component_collider_t *)
    );
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

    return 0;
}

void colliderbatchqueue_upload_to_jolt(colliderbatchqueue_t *const self)
{
    if (self->array_buffer.top == -1) return;

    qsort(self->array_buffer.data.raw_data, (u64)(self->array_buffer.top + 1), sizeof(ecs_component_collider_t *), colliderbatchqueue__internal_qsort_compare);

    JPH_Shape *shape                        = NULL;
    JPH_BodyCreationSettings *body_settings = NULL;
    JPH_CharacterVirtualSettings settings   = {0};
    JPH_MotionType prev_motion_type         = 0;

    for (i32 idx = 0; idx <= self->array_buffer.top; idx++)
    {
        ecs_component_collider_t *const collider    = ((ecs_component_collider_t **)self->array_buffer.data.raw_data)[idx];
        const bool diff_motion_type                 = prev_motion_type != collider->motion_type;

        //NOTE: we club all static motion types together before moving on to other motion types, this way we would
        //know when to optimize the broadphase i.e. after all static types are completely done.
        //  *   Added logic to run the optimize broadphase if the last remainig item is a static object and no other are left 
        //      afterwards of a different motion type (edge case)
        const bool optimize_broadphase = (diff_motion_type && prev_motion_type == JPH_MotionType_Static) 
            || ((idx == self->array_buffer.top) && prev_motion_type == JPH_MotionType_Static);

        switch(collider->shape_type)
        {
            case COLLIDER_SHAPE_TYPE_CUBE:
                shape = (JPH_Shape *)JPH_BoxShape_Create((JPH_Vec3 *)&collider->dim.cube, JPH_DEFAULT_CONVEX_RADIUS);
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

                vec3f_t *const scaled_vtx = arena_reserve(self->arena, vtx_count * sizeof(vec3f_t));
                for (u32 i = 0; i < vtx_count; i++)
                {
                    scaled_vtx[i].x = collider->dim.mesh.vtx.data[i].x * collider->internal.scale.x;
                    scaled_vtx[i].y = collider->dim.mesh.vtx.data[i].y * collider->internal.scale.y;
                    scaled_vtx[i].z = collider->dim.mesh.vtx.data[i].z * collider->internal.scale.z;
                }

                JPH_IndexedTriangle *const tris = arena_reserve(self->arena, tri_count * sizeof(JPH_IndexedTriangle));
                for (u32 i = 0; i < tri_count; i++) 
                {
                    tris[i] = (JPH_IndexedTriangle){
                        .i1 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 0] : i * 3 + 0,
                        .i2 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 1] : i * 3 + 1,
                        .i3 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 2] : i * 3 + 2,
                    };
                }

                JPH_MeshShapeSettings *const settings = JPH_MeshShapeSettings_Create2(
                    (const JPH_Vec3 *)scaled_vtx,
                    vtx_count,
                    tris,
                    tri_count
                );
                JPH_MeshShapeSettings_SetBuildQuality(settings, JPH_Mesh_Shape_BuildQuality_FavorRuntimePerformance);
                JPH_MeshShapeSettings_Sanitize(settings);
                shape = (JPH_Shape *)JPH_MeshShapeSettings_CreateShape(settings);
                JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)settings);

                arena_giveback(self->arena, scaled_vtx, vtx_count * sizeof(vec3f_t));
                arena_giveback(self->arena, tris, tri_count * sizeof(JPH_IndexedTriangle));
            } break;

            case COLLIDER_SHAPE_TYPE_SOFTBODY: {
                JPH_SoftBodySharedSettings* shared_settings = JPH_SoftBodySharedSettings_Create();
                const u32 vertex_count      = collider->dim.softbody.vtx.count;
                const u32 triangle_count    = collider->dim.softbody.idx.count / 3;
                const f32 *positions        = (f32 *)collider->dim.softbody.vtx.data;
                const u32 *indices          = collider->dim.softbody.idx.data;

                // Find max Y position (after scale applied)
                f32 max_y = -FLT_MAX;
                for (u32 i = 0; i < vertex_count; ++i) {
                    f32 y = positions[i * 3 + 1] * collider->internal.scale.y;
                    if (y > max_y) max_y = y;
                }

                // Find top vertices within epsilon of max_y
#define PIN_EPSILON 0.001f
                // Count how many vertices are at top
                u32 top_vertex_count = 0;
                for (u32 i = 0; i < vertex_count; ++i) {
                    f32 y = positions[i * 3 + 1] * collider->internal.scale.y;
                    if (fabsf(y - max_y) < PIN_EPSILON) top_vertex_count++;
                }
                //Step 2: Find leftmost/rightmost among top vertices
                // Find min/max X among top vertices
                f32 min_x = FLT_MAX, max_x = -FLT_MAX;
                for (u32 i = 0; i < vertex_count; ++i) {
                    f32 y = positions[i * 3 + 1] * collider->internal.scale.y;
                    if (fabsf(y - max_y) < PIN_EPSILON) {
                        f32 x = positions[i * 3 + 0] * collider->internal.scale.x;
                        if (x < min_x) min_x = x;
                        if (x > max_x) max_x = x;
                    }
                }

                for (u32 index = 0; index < vertex_count; ++index) 
                {
                    JPH_SoftVertex v;
                    v.position.x = positions[index * 3 + 0] * collider->internal.scale.x;
                    v.position.y = positions[index * 3 + 1] * collider->internal.scale.y;
                    v.position.z = positions[index * 3 + 2] * collider->internal.scale.z;
                    v.velocity = (JPH_Vec3){0, 0, 0};
                    v.invMass = 1.0f;

                    // Pin top-left and top-right corners
                    f32 y = v.position.y;
                    f32 x = v.position.x;
                    bool is_top = fabsf(y - max_y) < PIN_EPSILON;
                    bool is_left_corner = is_top && fabsf(x - min_x) < PIN_EPSILON;
                    bool is_right_corner = is_top && fabsf(x - max_x) < PIN_EPSILON;

                    v.invMass = (is_left_corner || is_right_corner) ? 0.0f : 1.0f;

                    JPH_SoftBodySharedSettings_AddVertex(shared_settings, &v);
                }

                for (u32 index = 0; index < triangle_count; ++index) 
                {
                    JPH_SoftFace face;
                    face.vertex1 = indices[index * 3 + 0];
                    face.vertex2 = indices[index * 3 + 1];
                    face.vertex3 = indices[index * 3 + 2];
                    face.materialIndex = 0;
                    JPH_SoftBodySharedSettings_AddFace(shared_settings, &face);
                }

                JPH_SoftBodySharedSettings_CreateConstraints(shared_settings, 1.0f, JPH_SoftBodyBendType_None);
                JPH_SoftBodySharedSettings_Optimize(shared_settings);

                JPH_Vec3 pos = {collider->internal.position.x, collider->internal.position.y, collider->internal.position.z};
                JPH_Quat rot = *(JPH_Quat *)&collider->internal.orientation;
                JPH_SoftBodyCreationSettings* creation_settings = JPH_SoftBodyCreationSettings_Create2(shared_settings, &pos, &rot, collider->object_layer_type);
                JPH_SoftBodyCreationSettings_SetNumIterations(creation_settings, 1);
                JPH_SoftBodyCreationSettings_SetUpdatePosition(creation_settings, true);
                JPH_SoftBodyCreationSettings_SetVertexRadius(creation_settings, 0.001f);
                JPH_SoftBodyCreationSettings_SetPressure(creation_settings, 0.0f);

                logging("Cloth: %u vertices, %u triangles", vertex_count, triangle_count);

                collider->internal.body_id = JPH_BodyInterface_CreateAndAddSoftBody(
                    global_joltphysics_instance->bodyinterface,
                    creation_settings,
                    JPH_Activation_Activate
                );

                JPH_SoftBodySharedSettings_Destroy(shared_settings);
                JPH_SoftBodyCreationSettings_Destroy(creation_settings);

                const ecs_collider_jolt_userdata_t userdata = {
                    .objectlayertype = collider->object_layer_type,
                    .entity_id = collider->internal.entity_id,
                };
                JPH_BodyInterface_SetUserData(
                    global_joltphysics_instance->bodyinterface,
                    collider->internal.body_id,
                    (u64)arena_store(self->arena, &userdata, sizeof(userdata))
                );

                //TODO: this need to refactored, this will bring confusion in the future if its kept here
                prev_motion_type  = collider->motion_type;
                continue;

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
                JPH_BodyCreationSettings_SetIsSensor(body_settings, collider->object_layer_type == POGGEN_RESERVED_OBJECT_LAYER_TYPE);
            break;
            case JPH_MotionType_Kinematic:
                JPH_CharacterVirtualSettings_Init(&settings);
                settings.base.shape = shape;
                settings.shapeOffset = (JPH_Vec3){ 0, collider->dim.capsule.half_height + collider->dim.capsule.radius, 0 };
                settings.characterPadding = 0.2f;
            break;
            default: eprint("Unknown motion type");
        }

        switch(collider->motion_type)
        {
            case JPH_MotionType_Static:
            case JPH_MotionType_Dynamic:
                ASSERT(body_settings);
                collider->internal.body_id = JPH_BodyInterface_CreateAndAddBody(
                    global_joltphysics_instance->bodyinterface, 
                    body_settings, 
                    collider->motion_type == JPH_MotionType_Static ? JPH_Activation_DontActivate : JPH_Activation_Activate
                );

                //WARN: keeping it here like this so as to keep it easy to make the futher 
                //expansion of `ecs_collider_jolt_userdata_t` whenever we may need to

                const ecs_collider_jolt_userdata_t userdata = {
                    .objectlayertype = collider->object_layer_type,
                    .entity_id = collider->internal.entity_id,
                };
                JPH_BodyInterface_SetUserData(
                    global_joltphysics_instance->bodyinterface,
                    collider->internal.body_id,
                    (u64)arena_store(self->arena, &userdata, sizeof(userdata))
                );
                JPH_BodyCreationSettings_Destroy(body_settings);
            break;
            case JPH_MotionType_Kinematic:
                collider->internal.kinematicbody = JPH_CharacterVirtual_Create(
                    &settings,
                    (JPH_RVec3 *)&collider->internal.position,
                    (JPH_Quat *) &collider->internal.orientation,
                    0,
                    global_joltphysics_instance->physics_system
                );
            break;
            default: eprint("motion type not accounted for");
        }

        if (optimize_broadphase) {
            JPH_PhysicsSystem_OptimizeBroadPhase(global_joltphysics_instance->physics_system);
            logging("optimizied broadphase");
        }

#ifndef DEBUG
        if (shape) {
            JPH_Shape_Destroy(shape);
            shape = NULL;
        }
#endif
        prev_motion_type  = collider->motion_type;
    }

    //NOTE: reseting array_buffer
    {
        self->array_buffer.top = -1;
        memset(self->array_buffer.data.raw_data, 0, self->array_buffer.data.size);
    }

}

#endif

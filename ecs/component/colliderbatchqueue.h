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
    stackarena_t *scratch_arena;

} colliderbatchqueue_t;


colliderbatchqueue_t        colliderbatchqueue(arena_t *const arena);
void                        colliderbatchqueue_add(colliderbatchqueue_t *const self, ecs_component_collider_t *const collider);
void                        colliderbatchqueue_upload_to_jolt(colliderbatchqueue_t *const self);


#ifndef IGNORE_COLLIDER_BATCH_QUEUE_IMPLEMENTATION

INTERNAL void colliderbatchqueue__internal__update_jolts_user_data(const ecs_component_collider_t *const collider, const u8 mesh_idx, arena_t *const arena)
{
    const ecs_collider_jolt_userdata_t userdata = {
        .objectlayertype = collider->object_layer_type,
        .entity_id = collider->internal.entity_id,
    };

    JPH_BodyInterface_SetUserData(
        global_joltphysics_instance->bodyinterface,
        collider->type == ECS_COLLIDER_RIGIDBODY ? collider->body.rigid.body_ids[mesh_idx] : collider->body.soft.bodyid,
        (u64)arena_store(arena, &userdata, sizeof(userdata))
    );
}

INTERNAL void colliderbatchqueue__internal__add_model_shape(ecs_component_collider_t *const collider, stackarena_t *const scratch_arena, arena_t *const persistent_arena)
{
    ASSERT(collider->motion_type == JPH_MotionType_Static);
    ASSERT(collider->dim.model.count <= ECS_COLLIDER_MAX_MESH_PER_MODEL);

    for (u8 mesh_idx = 0; mesh_idx < collider->dim.model.count; mesh_idx++)
    {
        const cgltf_accessor *const pos_accessor = collider->dim.model.vtx_list[mesh_idx];
        const cgltf_accessor *const idx_accessor = collider->dim.model.idx_list[mesh_idx];
        const bool has_indices = idx_accessor && idx_accessor->buffer_view && idx_accessor->buffer_view->buffer && idx_accessor->buffer_view->buffer->data;
        if (!has_indices) {
            eprint("given mesh didnot include indices during export, verify");
        }

        const u32 vtx_count = (u32)pos_accessor->count;
        const u32 idx_count = (u32)idx_accessor->count;
        const u32 tri_count = idx_count / 3;

        vec3f_t *const scaled_vtx = (vec3s *)stackarena_push(scratch_arena, vtx_count * sizeof(vec3f_t));
        for (u32 i = 0; i < vtx_count; i++)
        {
            f32 pos[3];
            cgltf_accessor_read_float(pos_accessor, i, pos, 3);

            scaled_vtx[i].x = pos[0] * collider->internal.scale.x;
            scaled_vtx[i].y = pos[1] * collider->internal.scale.y;
            scaled_vtx[i].z = pos[2] * collider->internal.scale.z;
        }

        JPH_IndexedTriangle *const tris = (JPH_IndexedTriangle *)stackarena_push(scratch_arena, tri_count * sizeof(JPH_IndexedTriangle));

        for (u32 i = 0; i < tri_count; i++) 
        {
            const cgltf_size i1 = cgltf_accessor_read_index(idx_accessor, i * 3 + 0);
            const cgltf_size i2 = cgltf_accessor_read_index(idx_accessor, i * 3 + 1);
            const cgltf_size i3 = cgltf_accessor_read_index(idx_accessor, i * 3 + 2);

            if (!(i1 < vtx_count && i2 < vtx_count && i3 < vtx_count)) {
                //NOTE: most likey why this happened again, is due to the blender export issue for gltf, 
                //try re exporting the model, but before that apply 'by loose parts' option on the model via Ctrl P in Edit mode after
                //all the mesh is the scene is selected.
                eprint("index value exceed vtx count value - 'check the note in code for the work around'");
            }

            tris[i] = (JPH_IndexedTriangle){
                .i1 = (uint32_t)i1,
                .i2 = (uint32_t)i2,
                .i3 = (uint32_t)i3,
                .materialIndex = 0,
                .userData = 0,
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
        JPH_Shape *const shape = (JPH_Shape *)JPH_MeshShapeSettings_CreateShape(settings);
        ASSERT(shape);
        JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)settings);

        stackarena_pop(scratch_arena);
        stackarena_pop(scratch_arena);


        JPH_BodyCreationSettings *const body_settings = JPH_BodyCreationSettings_Create3(
            shape,
            (JPH_Vec3 *)&collider->internal.position, 
            (JPH_Quat *)&collider->internal.orientation, 
            collider->motion_type, 
            collider->object_layer_type
        );
        JPH_BodyCreationSettings_SetIsSensor(body_settings, collider->object_layer_type == POGGEN_RESERVED_OBJECT_LAYER_TYPE);

        collider->body.rigid.body_ids[mesh_idx] = JPH_BodyInterface_CreateAndAddBody(
            global_joltphysics_instance->bodyinterface, 
            body_settings, 
            collider->motion_type == JPH_MotionType_Static ? JPH_Activation_DontActivate : JPH_Activation_Activate
        );

        colliderbatchqueue__internal__update_jolts_user_data(collider, mesh_idx, persistent_arena);

        JPH_BodyCreationSettings_Destroy(body_settings);
    }
}

INTERNAL void colliderbatchqueue__internal__add_softbody_shape(ecs_component_collider_t *const collider, arena_t *const arena)
{
    JPH_SoftBodySharedSettings* shared_settings = JPH_SoftBodySharedSettings_Create();
    const u32 vertex_count      = collider->dim.mesh.vtx.count;
    const u32 triangle_count    = collider->dim.mesh.idx.count / 3;
    const vec4s *positions      = (vec4s *)collider->dim.mesh.vtx.data;
    const u32 *indices          = collider->dim.mesh.idx.data;

    for (u32 index = 0; index < vertex_count; ++index) 
    {
        JPH_SoftVertex v;
        v.position.x = positions[index].x * collider->internal.scale.x;
        v.position.y = positions[index].y * collider->internal.scale.y;
        v.position.z = positions[index].z * collider->internal.scale.z;
        v.invMass    = positions[index].w;
        v.velocity   = (JPH_Vec3){0.0, 0.0, 0.0};

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

    JPH_SoftBodySharedSettings_CreateConstraints(shared_settings, 0.0f, JPH_SoftBodyBendType_Dihedral);

    // Skin the kinematic (invMass == 0) vertices onto the player's joints so the animation drives
    // them. Jolt interpolates the bind -> current skin position across the substeps, which avoids
    // the positional discontinuity that injecting positions per-frame caused.
    const u8    *joints       = collider->dim.mesh.vtx.joints;
    const vec4s *skin_weights = collider->dim.mesh.vtx.weights;
    const u32   joints_count  = collider->body.soft.joints_count;

    if (joints && skin_weights && joints_count > 0)
    {
        // Jolt computes skin_matrix = jointMatrices[jointIndex] * invBind. The application passes
        // the model-space glTF skin palette (joint * inverseBind) as jointMatrices, so an identity
        // inverse bind here reproduces the exact same skin matrix.
        const JPH_Mat4 identity_inv_bind = { .column = {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        }};

        for (u32 joint = 0; joint < joints_count; ++joint)
        {
            JPH_SoftInvBind inv_bind;
            inv_bind.jointIndex = joint;
            inv_bind.invBind    = identity_inv_bind;
            JPH_SoftBodySharedSettings_AddInvBind(shared_settings, &inv_bind);
        }

        for (u32 vtx_idx = 0; vtx_idx < vertex_count; ++vtx_idx)
        {
            if (positions[vtx_idx].w != 0.0f) continue;   // only hard-skin the pinned vertices

            JPH_SoftSkinned skinned = {0};
            skinned.vertex = vtx_idx;

            float total_weight = 0.0f;
            for (u32 k = 0; k < 4; ++k)
            {
                const float weight = ((const float *)&skin_weights[vtx_idx])[k];
                skinned.weights[k].invBindIndex = joints[vtx_idx * 4 + k];
                skinned.weights[k].weight       = weight;
                total_weight += weight;
            }

            // Jolt stops reading at the first zero weight. Guarantee the vertex still skins to
            // something rather than collapsing onto joint 0 / the origin.
            //FIXME: this never hit during my run, so what is the point in having this here for?
            if (total_weight <= 0.0f) {
                skinned.weights[0].invBindIndex = 0;
                skinned.weights[0].weight       = 1.0f;

            }

            skinned.maxDistance      = 0.0f;    // 0 => hard skin to the animated position
            skinned.backStopDistance = 0.0f;    // >= maxDistance => backstop disabled
            skinned.backStopRadius   = 0.0f;

            JPH_SoftBodySharedSettings_AddSkinnedConstraint(shared_settings, &skinned);
        }

        JPH_SoftBodySharedSettings_CalculateSkinnedConstraintNormals(shared_settings);
    }

    JPH_SoftBodySharedSettings_CalculateBendConstraintConstants(shared_settings);
    JPH_SoftBodySharedSettings_Optimize(shared_settings);

    const JPH_Vec3 pos = {0};
    const JPH_Quat rot = GLM_QUAT_IDENTITY_INIT;

    JPH_SoftBodyCreationSettings *creation_settings = JPH_SoftBodyCreationSettings_Create2(
        shared_settings, &pos, &rot, collider->object_layer_type
    );
    JPH_SoftBodyCreationSettings_SetNumIterations(creation_settings, 20);
    JPH_SoftBodyCreationSettings_SetUpdatePosition(creation_settings, false);
    JPH_SoftBodyCreationSettings_SetVertexRadius(creation_settings, 0.01f);
    JPH_SoftBodyCreationSettings_SetPressure(creation_settings, 0.0f);

    collider->body.soft.bodyid = JPH_BodyInterface_CreateAndAddSoftBody(
        global_joltphysics_instance->bodyinterface,
        creation_settings,
        JPH_Activation_Activate
    );

    JPH_SoftBodySharedSettings_Destroy(shared_settings);
    JPH_SoftBodyCreationSettings_Destroy(creation_settings);

    colliderbatchqueue__internal__update_jolts_user_data(collider, 0, arena);
}

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
        .arena = arena_init(arena),
        .scratch_arena = stackarena_init(ARENA_STARTING_BLOCK_SIZE * 10000),
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

        if (collider->type == ECS_COLLIDER_SOFTBODY) {
            //TODO: it be nice to have all shape types accounted for a softbody, so essentially
            ASSERT(collider->shape_type == COLLIDER_SHAPE_TYPE_MESH);
            colliderbatchqueue__internal__add_softbody_shape(collider, self->arena);
            prev_motion_type  = collider->motion_type;
            continue;
        }

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

                vec3f_t *const scaled_vtx = (vec3s *)stackarena_push(self->scratch_arena, vtx_count * sizeof(vec3f_t));
                for (u32 i = 0; i < vtx_count; i++)
                {
                    scaled_vtx[i].x = collider->dim.mesh.vtx.data[i].x * collider->internal.scale.x;
                    scaled_vtx[i].y = collider->dim.mesh.vtx.data[i].y * collider->internal.scale.y;
                    scaled_vtx[i].z = collider->dim.mesh.vtx.data[i].z * collider->internal.scale.z;
                }

                JPH_IndexedTriangle *const tris = (JPH_IndexedTriangle *)stackarena_push(self->scratch_arena, tri_count * sizeof(JPH_IndexedTriangle));
                for (u32 i = 0; i < tri_count; i++) 
                {
                    const uint32_t i1 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 0] : i * 3 + 0;
                    const uint32_t i2 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 1] : i * 3 + 1;
                    const uint32_t i3 = collider->dim.mesh.idx.data ? collider->dim.mesh.idx.data[i * 3 + 2] : i * 3 + 2;

                    if(!(i1 < vtx_count && i2 < vtx_count && i3 < vtx_count)) eprint("Mesh index out of bounds");

                    tris[i] = (JPH_IndexedTriangle){
                        .i1 = i1,
                        .i2 = i2,
                        .i3 = i3,
                        .materialIndex = 0,
                        .userData = 0,
                    };
                }

                JPH_MeshShapeSettings *const mesh_settings = JPH_MeshShapeSettings_Create2(
                    (const JPH_Vec3 *)scaled_vtx,
                    vtx_count,
                    tris,
                    tri_count
                );
                JPH_MeshShapeSettings_SetBuildQuality(mesh_settings, JPH_Mesh_Shape_BuildQuality_FavorRuntimePerformance);
                JPH_MeshShapeSettings_Sanitize(mesh_settings);
                shape = (JPH_Shape *)JPH_MeshShapeSettings_CreateShape(mesh_settings);
                JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)mesh_settings);

                stackarena_pop(self->scratch_arena);
                stackarena_pop(self->scratch_arena);
            } break;

            case COLLIDER_SHAPE_TYPE_MODEL:
                colliderbatchqueue__internal__add_model_shape(collider, self->scratch_arena, self->arena);
                prev_motion_type  = collider->motion_type;
                continue;
            break;
            default: eprint("Collider shape type not accounted for here");
        }

#ifdef DEBUG
        //NOTE: having this is very confusing, if scales are changed for CUBES during edit - new shape is generated but for 
        //CAPSULE and SPHERE is scaled up so for those alone we only need the original shape
        collider->body.rigid.shapes[0] = shape;
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

                collider->body.rigid.body_ids[0] = JPH_BodyInterface_CreateAndAddBody(
                    global_joltphysics_instance->bodyinterface, 
                    body_settings, 
                    collider->motion_type == JPH_MotionType_Static ? JPH_Activation_DontActivate : JPH_Activation_Activate
                );
                colliderbatchqueue__internal__update_jolts_user_data(collider, 0, self->arena);
                JPH_BodyCreationSettings_Destroy(body_settings);
            break;
            case JPH_MotionType_Kinematic:
                collider->body.rigid.kinematicbody = JPH_CharacterVirtual_Create(
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

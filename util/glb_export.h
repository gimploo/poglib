#pragma once
#include <poglib/basic.h>
#include <poglib/ecs.h>
#include <poglib/gfx/model/assimp.h>
#include <poglib/util/assetmanager.h>

#include <poglib/external/assimp/include/assimp/cexport.h>

bool glb_export_scene(arena_t *arena, ecs_t *ecs, u32 model_asset_id, str_t output_path);

#ifndef IGNORE_GLB_EXPORT_IMPLEMENTATION

#include <poglib/external/cglm/struct.h>

static mat4s glb_export__compose_matrix(vec3s pos, versors quat, vec3s scale)
{
    mat4s out = glms_mat4_identity();
    out = glms_translate(out, pos);
    out = glms_quat_rotate(out, quat);
    out = glms_scale(out, scale);
    return out;
}

static void glb_export__set_node_transform(struct aiNode *node, mat4s m)
{
    mat4s transposed = glms_mat4_transpose(m);
    memcpy(&node->mTransformation, &transposed.raw, sizeof(mat4s));
}

static mat4s glb_export__get_node_transform(const struct aiNode *node)
{
    mat4s out;
    memcpy(&out.raw, &node->mTransformation, sizeof(mat4s));
    return glms_mat4_transpose(out);
}

static void glb_export__traverse_node(
    struct aiNode *node,
    mat4s parent_world,
    vec3s *mesh_positions,
    versors *mesh_orientations,
    vec3s *mesh_scales,
    bool *mesh_found,
    u32 mesh_count)
{
    mat4s m = glb_export__get_node_transform(node);
    mat4s world = glms_mat4_mul(parent_world, m);

    if (node->mNumMeshes > 0)
    {
        mat4s parent_inv = glms_mat4_inv(parent_world);

        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            u32 mesh_idx = node->mMeshes[i];
            if (mesh_idx < mesh_count && mesh_found[mesh_idx])
            {
                mat4s new_world = glb_export__compose_matrix(
                    mesh_positions[mesh_idx],
                    mesh_orientations[mesh_idx],
                    mesh_scales[mesh_idx]);
                mat4s new_m = glms_mat4_mul(parent_inv, new_world);
                glb_export__set_node_transform(node, new_m);
                break;
            }
        }
    }

    for (u32 i = 0; i < node->mNumChildren; i++)
    {
        glb_export__traverse_node(node->mChildren[i], world, mesh_positions, mesh_orientations, mesh_scales, mesh_found, mesh_count);
    }
}

bool glb_export_scene(arena_t *arena, ecs_t *ecs, u32 model_asset_id, str_t output_path)
{
    const glmodel_t *model = (glmodel_t *)assetmanager_get_assetresource(
        &global_engine->systems.assets, ASSET_TYPE_MODEL, model_asset_id);
    if (!model || !model->scene || !model->scene->mRootNode) return false;

    struct aiScene *scene = model->scene;
    u32 mesh_count = scene->mNumMeshes;
    if (mesh_count == 0) return false;

    vec3s *mesh_positions = arena_reserve(arena, mesh_count * sizeof(vec3s));
    versors *mesh_orientations = arena_reserve(arena, mesh_count * sizeof(versors));
    vec3s *mesh_scales = arena_reserve(arena, mesh_count * sizeof(vec3s));
    bool *mesh_found = arena_reserve(arena, mesh_count * sizeof(bool));

    slot_iterator(&ecs->managers.entitymanager.entities, iter)
    {
        const ecs_entity_t *const entity = iter;
        if (!slot_iterator_index) continue;
        if (!(entity->component_signature & (ECS_CMP_MESH | ECS_CMP_TRANSFORM))) continue;

        const ecs_entity_query_t q = ecs_entity_query_components(ecs, entity->id, ECS_CMP_MESH | ECS_CMP_TRANSFORM);
        const ecs_component_mesh_t *mesh_cmp = q.entity_cmp_data[ECS_CMP_MESH_IDX];
        if (!mesh_cmp || !mesh_cmp->is_scene_instanced) continue;
        if (mesh_cmp->asset_id != model_asset_id) continue;
        if (mesh_cmp->mesh_idx >= mesh_count) continue;

        const ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        mesh_found[mesh_cmp->mesh_idx] = true;
        mesh_positions[mesh_cmp->mesh_idx] = (vec3s){ .x = t->position.x, .y = t->position.y, .z = t->position.z };
        mesh_orientations[mesh_cmp->mesh_idx] = (versors){ .x = t->orientation.x, .y = t->orientation.y, .z = t->orientation.z, .w = t->orientation.w };
        mesh_scales[mesh_cmp->mesh_idx] = (vec3s){ .x = t->scale.x, .y = t->scale.y, .z = t->scale.z };
    }

    bool any_found = false;
    for (u32 i = 0; i < mesh_count; i++)
    {
        if (mesh_found[i]) { any_found = true; break; }
    }

    if (!any_found) return false;

    mat4s root_world = glb_export__get_node_transform(scene->mRootNode);

    for (u32 i = 0; i < scene->mRootNode->mNumChildren; i++)
    {
        glb_export__traverse_node(
            scene->mRootNode->mChildren[i],
            root_world,
            mesh_positions,
            mesh_orientations,
            mesh_scales,
            mesh_found,
            mesh_count);
    }

    aiReturn result = aiExportScene(scene, "glb2", output_path.data, 0);

    return result == AI_SUCCESS;
}

#endif

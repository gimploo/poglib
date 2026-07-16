#pragma once
#include <poglib/basic.h>
#include <poglib/ecs.h>
#include <poglib/gfx/model/assimp.h>
#include <poglib/util/assetmanager.h>

#include <poglib/external/assimp/include/assimp/cexport.h>

bool glb_export_scene(ecs_t *ecs, u32 model_asset_id, str_t output_path);

#ifndef IGNORE_GLB_EXPORT_IMPLEMENTATION

#include <poglib/external/cglm/mat4.h>
#include <poglib/external/cglm/vec3.h>
#include <poglib/external/cglm/vec4.h>
#include <poglib/external/cglm/quat.h>
#include <poglib/external/cglm/affine.h>

static void glb_export__compose_matrix(float *out, float *pos, float *quat, float *scale)
{
    glm_mat4_identity(out);
    glm_translate(out, pos);
    glm_quat_rotate(out, quat, out);
    glm_scale(out, scale);
}

static void glb_export__set_node_transform(struct aiNode *node, float *m)
{
    float transposed[16];
    glm_mat4_transpose_to(m, transposed);
    memcpy(&node->mTransformation, transposed, sizeof(struct aiMatrix4x4));
}

static void glb_export__get_node_transform(float *out, const struct aiNode *node)
{
    memcpy(out, &node->mTransformation, sizeof(struct aiMatrix4x4));
    glm_mat4_transpose(out);
}

static void glb_export__traverse_node(
    struct aiNode *node,
    float *parent_world,
    float (*mesh_positions)[3],
    float (*mesh_orientations)[4],
    float (*mesh_scales)[3],
    bool *mesh_found,
    u32 mesh_count)
{
    float m[16];
    float world[16];
    glb_export__get_node_transform(m, node);
    glm_mat4_mul(parent_world, m, world);

    if (node->mNumMeshes > 0)
    {
        float parent_inv[16];
        glm_mat4_inv(parent_world, parent_inv);

        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            u32 mesh_idx = node->mMeshes[i];
            if (mesh_idx < mesh_count && mesh_found[mesh_idx])
            {
                float new_world[16];
                glb_export__compose_matrix(new_world,
                    mesh_positions[mesh_idx],
                    mesh_orientations[mesh_idx],
                    mesh_scales[mesh_idx]);
                float new_m[16];
                glm_mat4_mul(parent_inv, new_world, new_m);
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

bool glb_export_scene(ecs_t *ecs, u32 model_asset_id, str_t output_path)
{
    const glmodel_t *model = (glmodel_t *)assetmanager_get_assetresource(
        &global_engine->systems.assets, ASSET_TYPE_MODEL, model_asset_id);
    if (!model || !model->scene || !model->scene->mRootNode) return false;

    const struct aiScene *src_scene = model->scene;
    u32 mesh_count = src_scene->mNumMeshes;
    if (mesh_count == 0) return false;

    float (*mesh_positions)[3] = calloc(mesh_count, sizeof(float[3]));
    float (*mesh_orientations)[4] = calloc(mesh_count, sizeof(float[4]));
    float (*mesh_scales)[3] = calloc(mesh_count, sizeof(float[3]));
    bool *mesh_found = calloc(mesh_count, sizeof(bool));
    if (!mesh_positions || !mesh_orientations || !mesh_scales || !mesh_found)
    {
        free(mesh_positions);
        free(mesh_orientations);
        free(mesh_scales);
        free(mesh_found);
        return false;
    }

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
        {
            float pos[3]; pos[0] = t->position.x; pos[1] = t->position.y; pos[2] = t->position.z;
            glm_vec3_copy(pos, mesh_positions[mesh_cmp->mesh_idx]);
        }
        {
            float ori[4]; ori[0] = t->orientation.x; ori[1] = t->orientation.y; ori[2] = t->orientation.z; ori[3] = t->orientation.w;
            glm_vec4_copy(ori, mesh_orientations[mesh_cmp->mesh_idx]);
        }
        {
            float scl[3]; scl[0] = t->scale.x; scl[1] = t->scale.y; scl[2] = t->scale.z;
            glm_vec3_copy(scl, mesh_scales[mesh_cmp->mesh_idx]);
        }
    }

    bool any_found = false;
    for (u32 i = 0; i < mesh_count; i++)
    {
        if (mesh_found[i]) { any_found = true; break; }
    }

    if (!any_found)
    {
        free(mesh_positions);
        free(mesh_orientations);
        free(mesh_scales);
        free(mesh_found);
        return false;
    }

    struct aiScene *copy_scene = NULL;
    aiCopyScene(src_scene, &copy_scene);
    if (!copy_scene)
    {
        free(mesh_positions);
        free(mesh_orientations);
        free(mesh_scales);
        free(mesh_found);
        return false;
    }

    float root_world[16];
    glb_export__get_node_transform(root_world, copy_scene->mRootNode);

    for (u32 i = 0; i < copy_scene->mRootNode->mNumChildren; i++)
    {
        glb_export__traverse_node(
            copy_scene->mRootNode->mChildren[i],
            root_world,
            mesh_positions,
            mesh_orientations,
            mesh_scales,
            mesh_found,
            mesh_count);
    }

    aiReturn result = aiExportScene(copy_scene, "glb2", output_path.data, 0);

    aiFreeScene(copy_scene);
    free(mesh_positions);
    free(mesh_orientations);
    free(mesh_scales);
    free(mesh_found);

    return result == AI_SUCCESS;
}

#endif

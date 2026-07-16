#pragma once
#include <poglib/basic.h>
#include <poglib/ecs.h>
#include <poglib/gfx/model/assimp.h>
#include <poglib/util/assetmanager.h>

bool glb_export_scene(arena_t *arena, ecs_t *ecs, u32 model_asset_id, str_t output_path);

#ifndef IGNORE_GLB_EXPORT_IMPLEMENTATION

#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include <poglib/external/cgltf/cgltf.h>
#undef CGLTF_IMPLEMENTATION
#include <poglib/external/cgltf/cgltf_write.h>

INTERNAL void glb_export__set_trs(cgltf_node *node, vec3s pos, versors quat, vec3s scale)
{
    node->has_translation = true;
    node->translation[0] = pos.x;
    node->translation[1] = pos.y;
    node->translation[2] = pos.z;

    node->has_rotation = true;
    node->rotation[0] = quat.x;
    node->rotation[1] = quat.y;
    node->rotation[2] = quat.z;
    node->rotation[3] = quat.w;

    node->has_scale = true;
    node->scale[0] = scale.x;
    node->scale[1] = scale.y;
    node->scale[2] = scale.z;

    node->has_matrix = false;
}

INTERNAL void glb_export__decompose(const cgltf_float *m, vec3s *pos, versors *quat, vec3s *scale)
{
    pos->x = m[12]; pos->y = m[13]; pos->z = m[14];

    scale->x = sqrtf(m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);
    scale->y = sqrtf(m[4]*m[4] + m[5]*m[5] + m[6]*m[6]);
    scale->z = sqrtf(m[8]*m[8] + m[9]*m[9] + m[10]*m[10]);

    if (scale->x < 1e-8f) scale->x = 1e-8f;
    if (scale->y < 1e-8f) scale->y = 1e-8f;
    if (scale->z < 1e-8f) scale->z = 1e-8f;

    mat4s rot = glms_mat4_identity();
    rot.raw[0][0] = m[0] / scale->x; rot.raw[0][1] = m[1] / scale->x; rot.raw[0][2] = m[2] / scale->x;
    rot.raw[1][0] = m[4] / scale->y; rot.raw[1][1] = m[5] / scale->y; rot.raw[1][2] = m[6] / scale->y;
    rot.raw[2][0] = m[8] / scale->z; rot.raw[2][1] = m[9] / scale->z; rot.raw[2][2] = m[10] / scale->z;

    *quat = glms_mat4_quat(rot);
}

bool glb_export_scene(arena_t *arena, ecs_t *ecs, u32 model_asset_id, str_t output_path)
{
    const glmodel_t *model = (glmodel_t *)assetmanager_get_assetresource(
        &global_engine->systems.assets, ASSET_TYPE_MODEL, model_asset_id);
    if (!model || !model->filepath.data) return false;

    cgltf_options options = { .type = cgltf_file_type_glb };
    cgltf_data *data = NULL;
    if (cgltf_parse_file(&options, model->filepath.data, &data) != cgltf_result_success) return false;

    cgltf_size mesh_count = data->meshes_count;
    if (mesh_count == 0) { cgltf_free(data); return false; }

    vec3s *mesh_positions = arena_reserve(arena, mesh_count * sizeof(vec3s));
    versors *mesh_orientations = arena_reserve(arena, mesh_count * sizeof(versors));
    vec3s *mesh_scales = arena_reserve(arena, mesh_count * sizeof(vec3s));
    bool *mesh_found = arena_reserve(arena, mesh_count * sizeof(bool));

    ecs_componentmanager_t *cmp = &ecs->managers.componentmanager;
    slot_t *mesh_pool = slot_get_value(&cmp->componentpool_slots, ECS_CMP_MESH_IDX);

    slot_iterator(mesh_pool, iter)
    {
        const ecs_component_poolentry_t *entry = iter;
        const ecs_component_mesh_t *mesh_cmp = (ecs_component_mesh_t *)entry->entity_cmpdata;
        if (!entry->is_active || !mesh_cmp->is_scene_instanced) continue;
        if (mesh_cmp->asset_id != model_asset_id) continue;
        if (mesh_cmp->mesh_idx >= mesh_count) continue;

        ecs_entity_query_t q = ecs_componentmanager__internal__query_components(
            cmp, entry->entity_id, ECS_CMP_TRANSFORM);
        const ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        if (!t) continue;

        mesh_found[mesh_cmp->mesh_idx] = true;
        mesh_positions[mesh_cmp->mesh_idx] = (vec3s){ .x = t->position.x, .y = t->position.y, .z = t->position.z };
        mesh_orientations[mesh_cmp->mesh_idx] = (versors){ .x = t->orientation.x, .y = t->orientation.y, .z = t->orientation.z, .w = t->orientation.w };
        mesh_scales[mesh_cmp->mesh_idx] = (vec3s){ .x = t->scale.x, .y = t->scale.y, .z = t->scale.z };
    }

    {
        bool any_found = false;
        for (cgltf_size i = 0; i < mesh_count; i++)
        {
            if (mesh_found[i]) { any_found = true; break; }
        }
        if (!any_found) { cgltf_free(data); return false; }
    }

    for (cgltf_size i = 0; i < data->nodes_count; i++)
    {
        cgltf_node *node = &data->nodes[i];
        if (!node->mesh) continue;

        cgltf_size mesh_idx = (cgltf_size)(node->mesh - data->meshes);
        if (mesh_idx >= mesh_count || !mesh_found[mesh_idx]) continue;

        mat4s entity_world;
        entity_world = glms_mat4_identity();
        entity_world = glms_translate(entity_world, mesh_positions[mesh_idx]);
        entity_world = glms_quat_rotate(entity_world, mesh_orientations[mesh_idx]);
        entity_world = glms_scale(entity_world, mesh_scales[mesh_idx]);

        if (node->parent)
        {
            cgltf_float parent_world_f[16];
            mat4s parent_world;
            mat4s parent_inv;
            mat4s local_m;
            vec3s pos;
            versors quat;
            vec3s scale;

            cgltf_node_transform_world(node->parent, parent_world_f);
            memcpy(&parent_world.raw, parent_world_f, sizeof(mat4s));
            parent_inv = glms_mat4_inv(parent_world);
            local_m = glms_mat4_mul(parent_inv, entity_world);
            glb_export__decompose((const cgltf_float *)&local_m.raw, &pos, &quat, &scale);
            glb_export__set_trs(node, pos, quat, scale);
        }
        else
        {
            vec3s pos;
            versors quat;
            vec3s scale;

            glb_export__decompose((const cgltf_float *)&entity_world.raw, &pos, &quat, &scale);
            glb_export__set_trs(node, pos, quat, scale);
        }
    }

    options.type = cgltf_file_type_glb;
    cgltf_result result = cgltf_write_file(&options, output_path.data, data);

    cgltf_free(data);
    return result == cgltf_result_success;
}

#endif

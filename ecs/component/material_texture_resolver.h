#pragma once
#include "poglib/ecs/component.h"
#include "poglib/poggen.h"


vec4s               ecs_material_get_basecolor_for_mesh(const ecs_component_material_t *const material, const u8 mesh_idx);
gltexturelist_t     ecs_material_get_texturelist_for_mesh(const ecs_component_material_t *const material, const u8 mesh_idx);


vec4s ecs_material_get_basecolor_for_mesh(const ecs_component_material_t *const material, const u8 mesh_idx)
{
    if (!material->meshidx_to_texture_lookup.count) return COLOR_WHITE;
    return material->meshidx_to_texture_lookup.data[mesh_idx].basecolor;
}

gltexturelist_t ecs_material_get_texturelist_for_mesh(const ecs_component_material_t *const material, const u8 mesh_idx)
{
    gltexturelist_t texturelist = {0};
    if (!material->meshidx_to_texture_lookup.count) return texturelist;

    if (material->meshidx_to_texture_lookup.data[mesh_idx].basecolor_texture_id != INVALID_ASSET_ID)
    {
        gltexture2d_t *texture = (gltexture2d_t *)assetmanager_get_assetresource(
            &global_engine->systems.assets, ASSET_TYPE_TEXTURE, 
            material->meshidx_to_texture_lookup.data[mesh_idx].basecolor_texture_id
        );

        texturelist = (gltexturelist_t){
            .count = 1,
            .items = {
                [0] = (gltextureitem_t){
                    .tex_id = texture->id,
                }
            }
        };
    }
    return texturelist;
}

void ecs_system_material__internal__resolve_textures_from_model(ecs_component_material_t *material, ecs_componentmanager_t *manager, const u32 entity_id)
{
    if (material->meshidx_to_texture_lookup.count) return;

    const ecs_entity_query_t view = ecs_componentmanager__internal__query_components(manager, entity_id, ECS_CMP_MODEL | ECS_CMP_MESH);
    const ecs_component_model_t *model_cmp  = view.entity_cmp_data[ECS_CMP_MODEL_IDX];
    const ecs_component_mesh_t *mesh_cmp    = view.entity_cmp_data[ECS_CMP_MESH_IDX];

    ASSERT(model_cmp || mesh_cmp);

    u32 asset_id = 0;
    u32 mesh_idx = 0;
    if (model_cmp){ 
        mesh_idx = model_cmp->mesh_idx;
        asset_id = model_cmp->asset_id;
    }
    if (mesh_cmp) {
        mesh_idx = mesh_cmp->mesh_idx;
        asset_id = mesh_cmp->asset_id;
    }

    ASSERT(asset_id);

    glmodel_t *model = (glmodel_t *)assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_MODEL, asset_id);
    gpu_asset_t *asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset_async(&global_engine->systems.assets, asset_id);
    if (!asset) return;

    const slot_t material_assetid_lookup = asset->material_assetid_lookup;
    if (!material_assetid_lookup.len) return;

    const cgltf_mesh *single_mesh = &model->data->meshes[mesh_idx];

    bool has_textures = false;
    mesh2texture_lookup_t primmesh2texture_lookup = {0};
    for(u32 prim_idx = 0; prim_idx < single_mesh->primitives_count; prim_idx++)
    {
        const cgltf_material *material = single_mesh->primitives[prim_idx].material;
        if (material && material->has_pbr_metallic_roughness) 
        {
            const vec4s color = *(vec4s *)material->pbr_metallic_roughness.base_color_factor;
            primmesh2texture_lookup.data[prim_idx].basecolor = color;

            if (material->pbr_metallic_roughness.base_color_texture.texture) {
                const u64 material_index = material - model->data->materials;
                const u64 basecolor_texture_id = *(u64 *)slot_get_value(&material_assetid_lookup, material_index);
                primmesh2texture_lookup.data[prim_idx].basecolor_texture_id = basecolor_texture_id;
                has_textures = true;
            }
        }
        primmesh2texture_lookup.count++;
        ASSERT(primmesh2texture_lookup.count <= ARRAY_LEN(primmesh2texture_lookup.data));
    }

    material->meshidx_to_texture_lookup = primmesh2texture_lookup;
    material->internal.has_any_textures = has_textures;
}



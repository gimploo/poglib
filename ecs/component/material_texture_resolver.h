#pragma once
#include "poglib/poggen.h"


vec4s               ecs_material_get_basecolor_for_mesh(const ecs_component_material_t *const material, const u8 mesh_idx);
gltexturelist_t     ecs_material_get_texturelist_for_mesh(const ecs_component_material_t *const material, const u8 mesh_idx);


vec4s ecs_material_get_basecolor_for_mesh(const ecs_component_material_t *const material, const u8 mesh_idx)
{
    if (!material->meshidx_to_texture_lookup.count) return COLOR_WHITE;

    return material->meshidx_to_texture_lookup.data[mesh_idx].basecolor.a == 0.0f 
        ? COLOR_WHITE 
        : material->meshidx_to_texture_lookup.data[mesh_idx].basecolor;
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
                    .source = { 
                        .normal_texture = texture 
                    }
                }
            }
        };
    }
    return texturelist;
}




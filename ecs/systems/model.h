#pragma once
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/glcamera.h"

void ecs_system__internal_render_model(
    const glmodel_t *model, 
    const glshader_t *shader, 
    const matrix4f_t perspective_projection,
    glcamera_t *world_camera,
    const matrix4f_t transform,
    const bool wireframe_mode);

void ecs_system_model(const slot_t *const componentpool, const ecs_componentmanager_t * const cmp_manager)
{
    ASSERT(global_poggen);
    slot_iterator(componentpool, ITER)
    {
        const asset_id assetid = ((ecs_component_model_t *)ITER)->asset_id;
        const glmodel_t *model = (glmodel_t *)assetmanager_get_assetresource(
            &global_poggen->systems.assets, 
            ASSET_TYPE_MODEL,
            assetid
        );


        if (!model) {
            continue;
        }

        const u32 entity_id                         = ((ecs_component_model_t *)ITER)->internal.entity_id;
        const ecs_component_material_t *material    = (ecs_component_material_t *)ecs_componentmanager_get_component(
                cmp_manager, entity_id, ECS_CMP_MATERIAL
        );
        ASSERT(material);

        const glshader_t *shader = (glshader_t *)assetmanager_get_assetresource(
                &global_poggen->systems.assets, ASSET_TYPE_SHADER, material->shaderid);
        ASSERT(shader);

        const ecs_component_transform_t *transform = (ecs_component_transform_t *)ecs_componentmanager_get_component(
                cmp_manager, entity_id, ECS_CMP_TRANSFORM);
        ASSERT(transform);

        //ecs_system__internal_render_model(model, shader, projection, world_camera, transform->translation, false);

        //FIXME: this doesnt work out - bring renderqueue in here - but first decouple shader binding 
        //in glrenderer3d - depending on the geometry that is about to be rendered the renderqueue would
        //bind shaders as this would avoid multiple binding of the shader in multiple meshses that uses it
        //and avoid complexity of knowing how to bring glcamera and projection data within an ecs setting as it
        //brings unneeded complexity - lets push most of the rendering complexity into the renderqueue of sorting
        //common things together and have the ecs purely provide commands to it.
    }
}

void ecs_system__internal_render_model(
    const glmodel_t *model, 
    const glshader_t *shader, 
    const matrix4f_t perspective_projection,
    glcamera_t *world_camera,
    const matrix4f_t transform,
    const bool wireframe_mode)
{
    const glshaderconfig_t shaderconfigs[2] = {
        [0] = {
            .shader = shader,
            .uniforms = {
                .count = 8,
                .uniform = {
                    [0] = {
                        .name = "view",
                        .type = "matrix4f_t",
                        .value = glcamera_getview(world_camera)
                    },
                    [1] = {
                        .name = "projection", 
                        .type = "matrix4f_t", 
                        .value = perspective_projection
                    },
                    [2] = {
                        .name = "transform",
                        .type = "matrix4f_t",
                        .value = transform
                    },
                    [3] = {
                        .name = "material.color",
                        .type = "vec4f_t",
                        .value.vec4 = *(vec4f_t *)list_get_value(&model->colors, 0)
                    },
                    [4] = {
                        .name = "uBones", 
                        .type = "matrix4f_t []",
                        .value.mat4s = {
                            .count = model->transforms[0].len, 
                            .data = (matrix4f_t *)model->transforms[0].data
                        }
                    },
                    [5] = {
                        .name = "light.color",
                        .type = "vec4f_t",
                        .value.vec4 = COLOR_WHITE
                    },
                    [6] = {
                        .name = "light.ambient",
                        .type = "f32",
                        .value.f32 = 1.0f
                    },
                    [7] = {
                        .name = "light.position",
                        .type = "vec3f_t",
                        .value.vec3 = vec3f(1.0f)
                    }
                }
            }
        },
        [1] = {
            .shader = shader,
            .uniforms = {
                .count = 8,
                .uniform = {
                    [0] = {
                        .name = "view",
                        .type = "matrix4f_t",
                        .value = glcamera_getview(world_camera)
                    },
                    [1] = {
                        .name = "projection", 
                        .type = "matrix4f_t", 
                        .value = perspective_projection
                    },
                    [2] = {
                        .name = "transform",
                        .type = "matrix4f_t",
                        .value = transform
                    },
                    [3] = {
                        .name = "material.color",
                        .type = "vec4f_t",
                        .value.vec4 = *(vec4f_t *)list_get_value(&model->colors, 1)
                    },
                    [4] = {
                        .name = "uBones", 
                        .type = "matrix4f_t []",
                        .value.mat4s = {
                            .count = model->transforms[1].len, 
                            .data = (matrix4f_t *)model->transforms[1].data
                        }
                    },
                    [5] = {
                        .name = "light.color",
                        .type = "vec4f_t",
                        .value.vec4 = COLOR_WHITE
                    },
                    [6] = {
                        .name = "light.ambient",
                        .type = "f32",
                        .value.f32 = 1.0f
                    },
                    [7] = {
                        .name = "light.position",
                        .type = "vec3f_t",
                        .value.vec3 = vec3f(1.0f)
                    }
                }
            }
        }
    };
    glrenderer3d_draw_model(
        model, 
        (glshaderconfiglist_t){
            .count = 2,
            .configs = {
                [0] = shaderconfigs[0],
                [1] = shaderconfigs[1]
            }
        }, 
        wireframe_mode
    );
}

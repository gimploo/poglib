#pragma once
#include <cstdio>
#include <poglib/basic.h>
#include "shader.h"
#include "texture2d.h"
#include "types.h"

#define CGLTF_IMPLEMENTATION
#include <poglib/external/cgltf/cgltf.h>

typedef enum glmesh_attribute_type {

    GLMESH_ATTRIBUTE_UNKNOWN        = 0,
    GLMESH_ATTRIBUTE_TYPE_POSITION,
    GLMESH_ATTRIBUTE_TYPE_NORMAL,
    GLMESH_ATTRIBUTE_TYPE_TANGENT,
    GLMESH_ATTRIBUTE_TYPE_JOINT,
    GLMESH_ATTRIBUTE_TYPE_WEIGHT,
    GLMESH_ATTRIBUTE_TYPE_TEXCOORD,
    GLMESH_ATTRIBUTE_TYPE_COLOR,
    GLMESH_ATTRIBUTE_TYPE_COUNT,

} glmesh_attribute_type ;

typedef struct glmesh_layout_t {
    glmesh_attribute_type type;     // Type of attribute
    u32 idx;                   // Optional index (for joint/weight/texcoord/color)
} glmesh_layout_t;

typedef struct glmodel_t {

    char    path[32];
    slot_t  meshes;
    
} glmodel_t ;


glmodel_t   glmodel_init(const char *path);
list_t      glmodel_get_mesh(const glmodel_t *self);
void        glmodel_destroy(glmodel_t *self);


#ifndef IGNORE_GL_MODEL_IMPLEMENTATION

/*
  typedef struct gs_gfxt_mesh_raw_data_t {
    uint16_t prim_count;
    size_t* vertex_sizes;
    size_t* index_sizes;
    void** vertices;
    void** indices;
} gs_gfxt_mesh_raw_data_t;
**/


glmodel_t glmodel_init(const char *path)
{
    glmodel_t output = {0};
    // Use cgltf like a boss
    cgltf_options cgltf_options = {};
    char file_ext[32] = {0};

    // Get file extension from path
    cstr_get_file_extension(path, file_ext);

    if (strcmp(file_ext, "gltf") == 0)      logging("GFXT:Loading GLTF: %s", path);
    else if (strcmp(file_ext, "glb") == 0)  logging("GFXT:Loading GLTF: %s", path);
    else                                    eprint("file extension `%s` not supported", file_ext);

    file_t file = file_init(path, "rb");
    assert(file.size > 0);
    u8 *file_contents = (u8 *)calloc(1, file.size);
    assert(file_contents);

    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&cgltf_options, file_contents, file.size, &data);
    free(file_contents);

    if (result != cgltf_result_success) {
        cgltf_free(data);
        eprint("GFXT:Mesh:LoadFromFile:Failed load gltf");
    }

    // Load buffers as well
    result = cgltf_load_buffers(&cgltf_options, data, path);
    if (result != cgltf_result_success) {
        cgltf_free(data);
        eprint("GFXT:Mesh:LoadFromFile:Failed to load buffers");
    }

    // Type of index data
    size_t index_element_size = 0;

    // Temporary structures
    list_t positions        = list_init(vec3f_t );
    list_t normals          = list_init(vec3f_t );
    list_t tangents         = list_init(vec3f_t );
    list_t colors           = list_init(vec3f_t );
    list_t uvs              = list_init(vec2f_t );
    list_t weights          = list_init(f32);
    list_t joints           = list_init(f32);
    list_t layouts          = list_init(glmesh_layout_t);
    bytebuffer_t v_data     = bytebuffer_init();
    bytebuffer_t i_data     = bytebuffer_init();
    matrix4f_t world_mat    = MATRIX4F_IDENTITY;

    // Allocate memory for buffers
    const u32 mesh_count    = data->meshes_count;
    output.meshes           = slot_init(mesh_count, glmodel_t );

    // For each node, for each mesh
    u32 i = 0;
    for (uint32_t _n = 0; _n < data->nodes_count; ++_n)
    {
        cgltf_node* node = &data->nodes[_n];
        if (node->mesh == NULL) continue;

        logging("Load mesh from node: %s", node->name);

        // Reset matrix
        world_mat = MATRIX4F_IDENTITY;

        // logging("i: %zu, r: %zu, t: %zu, s: %zu, m: %zu", i, node->has_rotation, node->has_translation, node->has_scale, node->has_matrix);

        // Not sure what "local transform" does, since world gives me the actual world result...probably for animation
        if (node->has_rotation || node->has_translation || node->has_scale) 
        {
            cgltf_node_transform_world(node, (float*)&world_mat);
        }

        // if (node->has_matrix)
        // {
        //  // Multiply local by world
        //  gs_mat4 tmp = gs_default_val();
        //  cgltf_node_transform_world(node, (float*)&tmp);
        //  world_mat = gs_mat4_mul(world_mat, tmp);
        // }

        // Do node mesh data
        cgltf_mesh* cmesh = node->mesh;
        {
            // Initialize mesh data
            glmesh_t * mesh = (glmesh_t *)slot_get_value(&output.meshes, i);
            mesh->__vertices = slot_init(cmesh->primitives_count, glvertex3d_t );

            bool printed = false;

            // For each primitive in mesh 
            for (uint32_t p = 0; p < cmesh->primitives_count; ++p)
            {
                cgltf_primitive* prim = &cmesh->primitives[p];

                // Clear temp data from previous use
                list_clear(&positions);
                list_clear(&normals);
                list_clear(&tangents);
                list_clear(&uvs);
                list_clear(&colors);
                list_clear(&weights);
                list_clear(&joints);
                list_clear(&layouts);
                bytebuffer_clear(&v_data);
                bytebuffer_clear(&i_data);
                // Collect all provided attribute data for each vertex that's available in gltf data
                #define __GFXT_GLTF_PUSH_ATTR(ATTR, TYPE, COUNT, ARR, ARR_TYPE, LAYOUTS, LAYOUT_TYPE)\
                    do {\
                        int32_t N = 0;\
                        TYPE* BUF = (TYPE*)ATTR->buffer_view->buffer->data + ATTR->buffer_view->offset/sizeof(TYPE) + ATTR->offset/sizeof(TYPE);\
                        assert(BUF);\
                        TYPE V[COUNT] = {0};\
                        /* For each vertex */\
                        for (uint32_t k = 0; k < ATTR->count; k++)\
                        {\
                            /* For each element */\
                            for (int l = 0; l < COUNT; l++) {\
                                V[l] = BUF[N + l];\
                            }\
                            N += (int32_t)(ATTR->stride/sizeof(TYPE));\
                            /* Add to temp data array */\
                            ARR_TYPE ELEM = {0};\
                            memcpy((void*)&ELEM, (void*)V, sizeof(ARR_TYPE));\
                            list_append(&ARR, ELEM);\
                        }\
                        /* Push into layout */\
                        glmesh_layout_t LAYOUT = {LAYOUT_TYPE , 0};\
                        list_append(&LAYOUTS, LAYOUT);\
                    } while (0)

                // For each attribute in primitive
                for (uint32_t a = 0; a < prim->attributes_count; ++a)
                {
                    // Accessor for attribute data
                    cgltf_accessor* attr = prim->attributes[a].data;

                    // Index for data
                    int32_t aidx = prim->attributes[a].index;

                    // Switch on type for reading data
                    switch (prim->attributes[a].type)
                    {
                        case cgltf_attribute_type_position: {
                            int32_t N = 0;
                            float* BUF = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
                            assert(BUF);
                            float V[3] = {0};
                            /* For each vertex */
                            for (uint32_t k = 0; k < attr->count; k++)
                            {
                                /* For each element */
                                for (int l = 0; l < 3; l++) {
                                    V[l] = BUF[N + l];
                                } 
                                N += (int32_t)(attr->stride/sizeof(float));
                                /* Add to temp data array */
                                vec3f_t ELEM = {0};
                                memcpy((void*)&ELEM, (void*)V, sizeof(vec3f_t ));
                                // Transform into world space
                                ELEM = glms_mat4_mulv3(world_mat, ELEM, 0);
                                list_append(&positions, ELEM);
                            }
                            /* Push into layout */
                            glmesh_layout_t LAYOUT = { GLMESH_ATTRIBUTE_TYPE_POSITION, 0 };
                            list_append(&layouts, LAYOUT);
                        } break;

                        case cgltf_attribute_type_normal: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 3, normals, vec3f_t , layouts, GLMESH_ATTRIBUTE_TYPE_NORMAL);
                        } break;

                        case cgltf_attribute_type_tangent: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 3, tangents, vec3f_t , layouts, GLMESH_ATTRIBUTE_TYPE_TANGENT);
                        } break;

                        case cgltf_attribute_type_texcoord: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 2, uvs, vec2f_t , layouts, GLMESH_ATTRIBUTE_TYPE_TEXCOORD);
                        } break;

                        case cgltf_attribute_type_color: {
                            // Need to parse color as sRGB then convert to gs_color_t
                            int32_t N = 0;
                            float* BUF = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
                            assert(BUF);
                            float V[3] = {0};
                            /* For each vertex */\
                            for (uint32_t k = 0; k < attr->count; k++)
                            {
                                /* For each element */
                                for (int l = 0; l < 3; l++) {
                                    V[l] = BUF[N + l];
                                }
                                N += (int32_t)(attr->stride/sizeof(float));
                                /* Add to temp data array */
                                vec4f_t ELEM = {0};
                                // Need to convert over now
                                ELEM.r = (uint8_t)(V[0] * 255.f);
                                ELEM.g = (uint8_t)(V[1] * 255.f);
                                ELEM.b = (uint8_t)(V[2] * 255.f);
                                ELEM.a = 255; 
                                list_append(&colors, ELEM);
                            }
                            /* Push into layout */
                            glmesh_layout_t LAYOUT = {
                                GLMESH_ATTRIBUTE_TYPE_COLOR, 0 };
                            list_append(&layouts, LAYOUT);
                        } break;

                        // Not sure what to do with these for now
                        case cgltf_attribute_type_joints: 
                        {
                            // Push into layout
                            glmesh_layout_t layout = { GLMESH_ATTRIBUTE_TYPE_JOINT };
                            list_append(&layouts, layout);
                        } break;

                        case cgltf_attribute_type_weights:
                        {
                            // Push into layout
                            glmesh_layout_t layout = { GLMESH_ATTRIBUTE_TYPE_WEIGHT };
                            list_append(&layouts, layout);
                        } break;

                        // Shouldn't hit here...   
                        default: {
                        } break;
                    }
                }

                // Indices for primitive
                cgltf_accessor* acc = prim->indices;

                #define __GFXT_GLTF_PUSH_IDX(BB, ACC, TYPE)\
                    do {\
                        i32 n = 0;\
                        TYPE* buf = (TYPE*)acc->buffer_view->buffer->data + acc->buffer_view->offset/sizeof(TYPE) + acc->offset/sizeof(TYPE);\
                        assert(buf);\
                        TYPE v = 0;\
                        /* For each index */\
                        for (u32 k = 0; k < acc->count; k++) {\
                            /* For each element */\
                            for (int l = 0; l < 1; l++) {\
                                v = buf[n + l];\
                            }\
                            n += (i32)(acc->stride/sizeof(TYPE));\
                            /* Add to temp positions array */\
                            switch (index_element_size) {\
                                case 0: bytebuffer_write(BB, u16, (u16)v); break;\
                                case 2: bytebuffer_write(BB, u16, (u16)v); break;\
                                case 4: bytebuffer_write(BB, u32, (u32)v); break;\
                            }\
                        }\
                    } while (0)

                // If indices are available
                if (acc) 
                {
                    switch (acc->component_type) 
                    {
                        case cgltf_component_type_r_8:   __GFXT_GLTF_PUSH_IDX(&i_data, acc, i8);  break;
                        case cgltf_component_type_r_8u:  __GFXT_GLTF_PUSH_IDX(&i_data, acc, u8);  break;
                        case cgltf_component_type_r_16:  __GFXT_GLTF_PUSH_IDX(&i_data, acc, i16); break;
                        case cgltf_component_type_r_16u: __GFXT_GLTF_PUSH_IDX(&i_data, acc, u16); break;
                        case cgltf_component_type_r_32u: __GFXT_GLTF_PUSH_IDX(&i_data, acc, u32); break;
                        case cgltf_component_type_r_32f: __GFXT_GLTF_PUSH_IDX(&i_data, acc, f32); break;

                        // Shouldn't hit here
                        default: {
                        } break;
                    }
                }
                else 
                {
                    // Iterate over positions size, then just push back indices
                    for (uint32_t i = 0; i < positions.len; ++i) 
                    {
                        switch (index_element_size)
                        {
                            default:
                            case 0: bytebuffer_write(&i_data, u16, (u16)i); break;
                            case 2: bytebuffer_write(&i_data, u16, (u16)i); break;
                            case 4: bytebuffer_write(&i_data, u32, (u32)i); break;
                        }
                    }
                }



                // Indices
                slot_t *indices = &mesh->__indices;
                *indices = slot_init(prim->indices->count, u32 );
                memcpy(indices->__data, i_data.data, i_data.size);


                // Positions
                if (positions.len != 0)
                {
                    slot_t *vertices = &mesh->__vertices;
                    memcpy(vertices->__data, positions.__data, positions.len * positions.__elem_size);
                }

                // Normals
                if (normals.len != 0)
                {
                    memcpy(mesh->__normals.__data, 
                            normals.__data, 
                            normals.len * normals.__elem_size);
                }

                // Tangents
                if (tangents.len != 0)
                {
                    memcpy(mesh->__tangents.__data, 
                            tangents.__data, 
                            tangents.len * tangents.__elem_size);
                }

                /*
                // Texcoords
                for (uint32_t tci = 0; tci < GS_GFXT_TEX_COORD_MAX; ++tci)
                {
                    if (!gs_dyn_array_empty(uvs[tci]))
                    {
                        primitive.tex_coords[tci].size = gs_dyn_array_size(uvs[tci]) * sizeof(gs_vec2);
                        primitive.tex_coords[tci].data = gs_malloc(primitive.tex_coords[tci].size);
                        memcpy(primitive.tex_coords[tci].data, uvs[tci], primitive.tex_coords[tci].size);
                    }
                    else
                    {
                        break;
                    }
                }

                // Colors
                for (uint32_t ci = 0; ci < GS_GFXT_COLOR_MAX; ++ci)
                {
                    if (!gs_dyn_array_empty(colors[ci]))
                    {
                        primitive.colors[ci].size = gs_dyn_array_size(colors[ci]) * sizeof(gs_color_t);
                        primitive.colors[ci].data = gs_malloc(primitive.colors[ci].size);
                        memcpy(primitive.colors[ci].data, colors[ci], primitive.colors[ci].size);
                    }
                    else
                    {
                        break;
                    }
                }

                // Joints
                for (uint32_t ji = 0; ji < GS_GFXT_JOINT_MAX; ++ji)
                {
                    if (!gs_dyn_array_empty(joints[ji]))
                    {
                        primitive.joints[ji].size = gs_dyn_array_size(joints[ji]) * sizeof(float);
                        primitive.joints[ji].data = gs_malloc(primitive.joints[ji].size);
                        memcpy(primitive.joints[ji].data, joints[ji], primitive.joints[ji].size);
                    }
                    else
                    {
                        break;
                    }
                }

                // Weights
                for (uint32_t wi = 0; wi < GS_GFXT_WEIGHT_MAX; ++wi)
                {
                    if (!gs_dyn_array_empty(weights[wi]))
                    {
                        primitive.weights[wi].size = gs_dyn_array_size(weights[wi]) * sizeof(float);
                        primitive.weights[wi].data = gs_malloc(primitive.weights[wi].size);
                        memcpy(primitive.weights[wi].data, weights[wi], primitive.weights[wi].size);
                    }
                    else
                    {
                        break;
                    }
                }
                */
            } 

            /*if (!printed)*/
            /*{*/
                /*printed = true;*/
                /*if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION]){*/
                    /*gs_log_warning("Mesh attribute: POSITION not found. Resorting to default."); */
                /*}*/

                /*if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD]) {*/
                    /*gs_log_warning("Mesh attribute: TEXCOORD not found. Resorting to default."); */
                /*}*/

                /*if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR]) {*/
                    /*gs_log_warning("Mesh attribute: COLOR not found. Resorting to default."); */
                /*}*/

                /*if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL]) {*/
                    /*gs_log_warning("Mesh attribute: NORMAL not found. Resorting to default."); */
                /*}*/

                /*if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT]) {*/
                    /*gs_log_warning("Mesh attribute: WEIGHTS not found. Resorting to default."); */
                /*} */
            /*} */
        }

        // Increment i if successful
        i++;
    }

    printf("Finished loading mesh.");

    // Free all data at the end
    cgltf_free(data);
    list_destroy(&positions);
    list_destroy(&normals);
    list_destroy(&tangents);
    /*for (uint32_t ci = 0; ci < GS_GFXT_COLOR_MAX; ++ci) gs_dyn_array_free(colors[ci]);*/
    /*for (uint32_t tci = 0; tci < GS_GFXT_TEX_COORD_MAX; ++tci) gs_dyn_array_free(uvs[tci]);*/
    /*for (uint32_t wi = 0; wi < GS_GFXT_WEIGHT_MAX; ++wi) gs_dyn_array_free(weights[wi]);*/
    /*for (uint32_t ji = 0; ji < GS_GFXT_JOINT_MAX; ++ji) gs_dyn_array_free(joints[ji]);*/
    list_destroy(&layouts);
    bytebuffer_destroy(&v_data);
    bytebuffer_destroy(&i_data);

    return output;
}


#endif

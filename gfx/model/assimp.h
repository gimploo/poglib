#pragma once
#include <poglib/basic.h>
#include "../gl/shader.h"
#include "../gl/texture2d.h"
#include "../gl/types.h"

#include <poglib/external/assimp/include/assimp/defs.h>
#include <poglib/external/assimp/include/assimp/importerdesc.h>
#include <poglib/external/assimp/include/assimp/cimport.h>
#include <poglib/external/assimp/include/assimp/mesh.h>
#include <poglib/external/assimp/include/assimp/scene.h>
#include <poglib/external/assimp/include/assimp/postprocess.h>
#include <poglib/external/assimp/include/assimp/material.h>

typedef struct glmodel_t {

    const char *filepath[64];
    list_t meshes;
    list_t textures;
    list_t colors;

} glmodel_t ;


glmodel_t   glmodel_init(const char *filepath);
void        glmodel_destroy(glmodel_t *self);


#ifndef IGNORE_ASSIMP_IMPLEMENTATION

void __glmesh_processMaterials(glmodel_t *self, const struct aiMaterial *material)
{
    struct { enum aiTextureType type; char *name; } textureTypes[] = {
        //{ aiTextureType_NONE, "texture_none" },
        //{ aiTextureType_UNKNOWN, "texture_unknown" },
        { aiTextureType_DIFFUSE, "texture_diffuse" },
        { aiTextureType_SPECULAR, "texture_specular" },
        { aiTextureType_AMBIENT, "texture_ambient" },
        { aiTextureType_HEIGHT, "texture_height" },
        { aiTextureType_NORMALS, "texture_normal" },
        { aiTextureType_EMISSIVE, "texture_emissive"},
        { aiTextureType_SHININESS, "texture_shininess"},
        { aiTextureType_OPACITY, "texture_opacity"},
        { aiTextureType_DISPLACEMENT, "texture_displacement"},
        { aiTextureType_LIGHTMAP, "texture_lightmap"},
        { aiTextureType_REFLECTION, "texture_reflection"},
        { aiTextureType_BASE_COLOR, "texture_base_color"},
        { aiTextureType_NORMAL_CAMERA, "texture_normal_camera"},
        { aiTextureType_EMISSION_COLOR, "texture_emission_color"},
        { aiTextureType_METALNESS, "texture_metalness"},
        { aiTextureType_DIFFUSE_ROUGHNESS, "texture_diffuse_roughness"},
        { aiTextureType_AMBIENT_OCCLUSION, "texture_occlusion"},
        { aiTextureType_SHEEN, "texture_sheen"},
        { aiTextureType_CLEARCOAT, "texture_clearcoat"},
        { aiTextureType_TRANSMISSION, "texture_transmission"},
#ifdef _WIN64
        { aiTextureType_MAYA_BASE, "texture_maya_base" },
        { aiTextureType_MAYA_SPECULAR, "texture_maya_specular" },
        { aiTextureType_MAYA_SPECULAR_COLOR, "texture_maya_specular_color" },
        { aiTextureType_MAYA_SPECULAR_ROUGHNESS, "texture_maya_specular_roughness" },
        { aiTextureType_ANISOTROPY, "texture_anisotropy" },
        { aiTextureType_GLTF_METALLIC_ROUGHNESS, "texture_gltf_metallic_roughness" },
#endif
    };

    struct aiColor4D color;
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
        list_append(&self->colors, color);
    }

    bool loaded_textures[AI_TEXTURE_TYPE_MAX] = {0};

    for (u32 textype_count = 0; textype_count < ARRAY_LEN(textureTypes); textype_count++) {

        if (loaded_textures[textureTypes[textype_count].type]) 
            continue;

        const u32 total_textures_for_single_type = aiGetMaterialTextureCount(material, textureTypes[textype_count].type);
        if (!total_textures_for_single_type)
            continue;

        ASSERT(total_textures_for_single_type == 1 && "We assumed that only a single texture is loaded per type at all times");

        struct aiString texture_filepath;
        aiGetMaterialTexture(
            material, 
            textureTypes[textype_count].type, 
            0, 
            &texture_filepath, 
            NULL, NULL, NULL, NULL, NULL, NULL);

        gltexture2d_t texture = gltexture2d_init(texture_filepath.data);
        list_append(&self->textures, texture);

        loaded_textures[textureTypes[textype_count].type] = true;
    }
}

glmesh_t __glmesh_processMesh(glmodel_t *self, struct aiMesh *mesh, const struct aiScene *scene)
{
    // get the total total indicies in the mesh
    u64 total_indicies = 0;
    for(u32 i = 0; i < mesh->mNumFaces; i++) 
        total_indicies += mesh->mFaces[i].mNumIndices;

    slot_t vtx = slot_init(mesh->mNumVertices, glvertex3d_t );
    slot_t ind = slot_init(total_indicies, u32);

    // Copy indices
    for(u32 i = 0; i < mesh->mNumFaces; i++)
    {
        struct aiFace face = mesh->mFaces[i];
        slot_insert_multiple(&ind, (const u8 *)face.mIndices, face.mNumIndices, sizeof(u32));
    }

    // Copy vertices, normals, UVs
    for (u32 i = 0; i < mesh->mNumVertices; i++)
    {
        ASSERT(mesh->mVertices && "Given mesh has no vertices");

        glvertex3d_t vt = {0};

        vt.pos = (vec3f_t ){
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z,
        };

        if (i < AI_MAX_NUMBER_OF_TEXTURECOORDS && mesh->mTextureCoords[i]) {
            vt.uv = (vec2f_t) {
                mesh->mTextureCoords[i]->x,
                mesh->mTextureCoords[i]->y,
            };
            vt.tangents = (vec3f_t) {
                .x = mesh->mTangents[i].x,
                .y = mesh->mTangents[i].y,
                .z = mesh->mTangents[i].z,
            };
            vt.bitangents = (vec3f_t) {
                .x = mesh->mBitangents[i].x,
                .y = mesh->mBitangents[i].y,
                .z = mesh->mBitangents[i].z,
            };
        }

        if (mesh->mNormals) {
            vt.norm = (vec3f_t){
                mesh->mNormals->x,
                mesh->mNormals->y,
                mesh->mNormals->z,
            };
        }

        slot_append(&vtx, vt);
    }

    //Process materials
    __glmesh_processMaterials(self, scene->mMaterials[mesh->mMaterialIndex]);

    return (glmesh_t) {
        .vtx = vtx,
        .idx = ind
    };
}


void __glmesh_processNode(glmodel_t *self, struct aiNode *node, const struct aiScene *scene)
{
    // process each mesh located at the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        glmesh_t m = __glmesh_processMesh(self, mesh, scene);
        list_append(&self->meshes, m);
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        __glmesh_processNode(self, node->mChildren[i], scene);
    }
}

glmodel_t glmodel_init(const char *filepath)
{
    ASSERT(strlen(filepath) < 64);

    glmodel_t o = {0};
    memcpy(o.filepath, filepath, strlen(filepath));
    o.meshes = list_init(glmesh_t );
    o.textures = list_init(gltexture2d_t );
    o.colors = list_init(vec4f_t );

    // Assimp: import model
    const struct aiScene* scene = aiImportFile(filepath,
       aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
    ); // http://assimp.sourceforge.net/lib_html/structai_scene.html
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        eprint("ERROR::ASSIMP:: %s", aiGetErrorString());
    }

    __glmesh_processNode(&o, scene->mRootNode, scene);
	
    // free scene
    aiReleaseImport(scene);

    return o;
}

void glmodel_destroy(glmodel_t *self)
{
    list_iterator(&self->meshes, iter) {
        glmesh_destroy((glmesh_t *)iter);
    }
    list_destroy(&self->meshes);

    list_iterator(&self->textures, iter) {
        gltexture2d_destroy(iter);
    }
    list_destroy(&self->textures);

    list_destroy(&self->colors);

    memset(self->filepath, 0, sizeof(self->filepath));
}


#endif

#pragma once
#include <poglib/basic.h>
#include "shader.h"
#include "texture2d.h"
#include "types.h"
#include <poglib/external/assimp/include/assimp/cimport.h>
#include <poglib/external/assimp/include/assimp/scene.h>
#include <poglib/external/assimp/include/assimp/postprocess.h>

typedef struct glmodel_t {

    const char      *filepath;
    const str_t     directory;
    list_t          meshes;
    list_t          textures;
    bool            gamma_correction;

} glmodel_t ;


glmodel_t   glmodel_init(const char *filepath);
list_t      glmodel_get_mesh(const glmodel_t *self);
void        glmodel_destroy(glmodel_t *self);


#ifndef IGNORE_GL_MODEL_IMPLEMENTATION

list_t __glmodel_loadMaterialTextures(
        glmodel_t *self,
        aiMaterial *mat, 
        aiTextureType type, 
        const char * typeName)
{
    list_t textures = list_init(gltexture2d_t );
    for(unsigned int i = 0; i < aiGetMaterialTextureCount(mat, type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        list_iterator(&self->textures, iter)
        {
            gltexture2d_t *texture = (gltexture2d_t *)iter; 

            if(std::strcmp(texture->filepath, str.data) == 0)
            {
                list_append(&textures, *texture);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            char fullpath[32] = {0};

            char dir[32] = {0};
            str_get_data(&self->directory, dir);
            cstr_combine_path(dir, str.data, fullpath);

            gltexture2d_t texture = gltexture2d_init(fullpath);
            cstr_copy((char *)texture.type, typeName);
            list_append(&textures, texture);
            list_append(&self->textures, texture);
        }
    }
    return textures;
}

glmesh_t __glmodel_processMesh(glmodel_t *self, aiMesh *mesh, const aiScene *scene)
{
    // data to fill
    list_t vertices = list_init(glvertex3d_t );
    list_t indices = list_init(u32);
    list_t textures = list_init(gltexture2d_t );

    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        glvertex3d_t vertex;
        vec3f_t vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        if (mesh->mNormals != NULL && mesh->mNumVertices > 0)
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        // texture coordinates
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            vec2f_t vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.tex_coords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        }
        else vertex.tex_coords = (vec2f_t ){0.0f, 0.0f};

        list_append(&vertices, vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            list_append(&indices, face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    list_t diffuseMaps = __glmodel_loadMaterialTextures(self, material, aiTextureType_DIFFUSE, "texture_diffuse");
    list_combine(&self->textures, &diffuseMaps);
    // 2. specular maps
    list_t specularMaps = __glmodel_loadMaterialTextures(self, material, aiTextureType_SPECULAR, "texture_specular");
    list_combine(&self->textures, &specularMaps);
    // 3. normal maps
    list_t normalMaps = __glmodel_loadMaterialTextures(self, material, aiTextureType_HEIGHT, "texture_normal");
    list_combine(&self->textures, &normalMaps);
    // 4. height maps
    list_t heightMaps = __glmodel_loadMaterialTextures(self, material, aiTextureType_AMBIENT, "texture_height");
    list_combine(&self->textures, &heightMaps);

    // return a mesh object created from the extracted mesh data
    return glmesh_init(vertices, indices, textures);
}

void __glmodel_processNode(glmodel_t *m, aiNode *node, const aiScene *scene)
{
    // process each mesh located at the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        glmesh_t tmp = __glmodel_processMesh(m, mesh, scene);
        list_append(&m->meshes, tmp);
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        __glmodel_processNode(m, node->mChildren[i], scene);
    }

}

glmodel_t glmodel_init(const char *filepath)
{
    assert(filepath);
    const aiScene *scene = aiImportFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        eprint("ERROR::ASSIMP:: %s ", aiGetErrorString());

    glmodel_t m = {
        .filepath = filepath,
        .directory = str_get_directory_path(filepath),
        .meshes = list_init(glmesh_t ), 
        .textures = list_init(gltexture2d_t )
    };

    __glmodel_processNode(&m, scene->mRootNode, scene);

    return m;
}

list_t glmodel_get_mesh(const glmodel_t *model)
{
    return model->meshes;
}

void glmodel_destroy(glmodel_t *self)
{
    list_iterator(&self->textures, iter)
        glmesh_destroy((glmesh_t *)iter);
    list_destroy(&self->meshes);

    list_iterator(&self->textures, iter)
        gltexture2d_destroy((gltexture2d_t *)iter);
    list_destroy(&self->textures);
}

#endif

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

typedef struct glmodel_t {

    const char *filepath[65];
    list_t meshes;

} glmodel_t ;


glmodel_t   glmodel_init(const char *filepath);
void        glmodel_destroy(glmodel_t *self);


#ifndef IGNORE_ASSIMP_IMPLEMENTATION

glmesh_t __glmesh_processMesh(glmodel_t *self, struct aiMesh *mesh, const struct aiScene *scene)
{
    // get the total total indicies in the mesh
    u64 total_indicies = 0;
    for(int i = 0; i < mesh->mNumFaces; i++) 
        total_indicies += mesh->mFaces[i].mNumIndices;

    slot_t vtx = slot_init(mesh->mNumVertices, glvertex3d_t );
    slot_t ind = slot_init(total_indicies, u32);

    // Copy indices
    for(int i = 0; i < mesh->mNumFaces; i++)
    {
        struct aiFace face = mesh->mFaces[i];
        slot_insert_multiple(&ind, face.mIndices, face.mNumIndices, sizeof(u32));
    }

    // Copy vertices, normals, UVs
    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        ASSERT(mesh->mVertices && "Given mesh has no vertices");

        glvertex3d_t vt = {0};

        vt.pos = (vec3f_t ){
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z,
        };

        if (mesh->mTextureCoords) {
            vt.uv = (vec2f_t) {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y,
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
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z,
            };
        }

        slot_append(&vtx, vt);
    }

    //Process materials
    //__glmesh_processMaterials(self, scene->mMaterials[mesh->mMaterialIndex]);

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
    glmodel_t o = {0};
    memcpy(o.filepath, filepath, strlen(filepath));
    o.meshes = list_init(glmesh_t );

    // Assimp: import model
    const struct aiScene* scene = aiImportFile(filepath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace); // http://assimp.sourceforge.net/lib_html/structai_scene.html
    
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

    memset(self->filepath, 0, sizeof(self->filepath));
}


#endif

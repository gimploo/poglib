#pragma once
#include <poglib/basic.h>
#include "../gl/shader.h"
#include "../gl/texture2d.h"
#include "../gl/types.h"

#include <assimp/defs.h>
#include <assimp/importerdesc.h>
#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef struct glmodel_t {

    const char *filepath[65];
    list_t meshes;

} glmodel_t ;


glmodel_t   glmodel_init(const char *filepath);
void        glmodel_destroy(glmodel_t *self);


#ifndef IGNORE_ASSIMP_IMPLEMENTATION

glmesh_t __glmesh_processMesh(glmodel_t *self, struct aiMesh *mesh, const struct aiScene *scene)
{
    slot_t vtx = slot_init(mesh->mNumVertices, glvertex3d_t );
    list_t ind = list_init(u32 );

    // copy indices from stupid array format
    for(int i = 0; i < mesh->mNumFaces; i++) 
    {
        struct aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            list_append(&ind, face.mIndices[j]);
    }

    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        const glvertex3d_t vt = {
            .pos = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z,
            },
            .norm = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z,
            },
            .uv = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y,
            },
        };
            
        slot_append(&vtx, vt);
    }

    return glmesh_init(vtx, ind);
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
                                                                                                                                                               //
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

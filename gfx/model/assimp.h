#pragma once
#include "../gl/texture2d.h"
#include "../gl/types.h"
#include <poglib/basic.h>
#include "./animation.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/ds/list.h"
#include "poglib/basic/str.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/texture_types.h"

#include <poglib/external/assimp/include/assimp/cimport.h>
#include <poglib/external/assimp/include/assimp/defs.h>
#include <poglib/external/assimp/include/assimp/importerdesc.h>
#include <poglib/external/assimp/include/assimp/material.h>
#include <poglib/external/assimp/include/assimp/mesh.h>
#include <poglib/external/assimp/include/assimp/postprocess.h>
#include <poglib/external/assimp/include/assimp/scene.h>

//TODO: materials implementation is fucked - need to learn how do this 
//TODO: arena support

#define MAX_BONES 128

typedef struct {
    matrix4f_t offset;
    matrix4f_t transform;
} boneinfo_t;

void print_boneinfo(void *data)
{
    boneinfo_t *b = data;
    matrix4f_print(&b->offset);
    matrix4f_print(&b->transform);
}

boneinfo_t boneinfo(matrix4f_t offset) {
    return (boneinfo_t ) {
        .offset = offset,
        .transform = MATRIX4F_IDENTITY
    };
}

//NOTE:
/*
1. Model File (Blender/FBX)  
   │ → Vertices/Bones defined in *model space*.  
   │ → Exported with root transform (e.g., scale=2.0).  
2. Assimp Loads:  
   │ → Applies root transform: *model space → world space*.  
3. Animation:  
   │ → Inverse root transform: *world space → model space*.  
   │ → Apply animations in *model space*.  
   │ → Transform back to world space (if needed).
*/

#define MAX_MESHES_PER_MODEL WORD

typedef struct glmodel_t {

    str_t directory_path;
    char *filepath[1024];
    list_t meshes;
    list_t textures;
    list_t colors;
    list_t bone_infos;
    //TODO: Implement materials;

    list_t transforms[MAX_MESHES_PER_MODEL];
    animator_t animator;
    hashtable_t *bone_name_to_index;

    const struct aiScene *scene;
    f32 current_time;

    matrix4f_t global_inverse_transform;
    arena_t arena;

} glmodel_t;

glmodel_t       glmodel_init(const char *filepath);
void            glmodel_set_animation(glmodel_t *self, const char *animation_label, const f32 dt, const bool loop);
void            glmodel_destroy(glmodel_t *self);

#ifndef IGNORE_ASSIMP_IMPLEMENTATION

const struct {
    enum aiTextureType type;
    char *name;
} ASSIMP_TEXTURE_TYPES[AI_TEXTURE_TYPE_MAX] = {
    //{ aiTextureType_NONE, "texture_none" },
    //{ aiTextureType_UNKNOWN, "texture_unknown" },
    {aiTextureType_DIFFUSE, "texture_diffuse"},
    {aiTextureType_SPECULAR, "texture_specular"},
    {aiTextureType_AMBIENT, "texture_ambient"},
    {aiTextureType_HEIGHT, "texture_height"},
    {aiTextureType_NORMALS, "texture_normal"},
    {aiTextureType_EMISSIVE, "texture_emissive"},
    {aiTextureType_SHININESS, "texture_shininess"},
    {aiTextureType_OPACITY, "texture_opacity"},
    {aiTextureType_DISPLACEMENT, "texture_displacement"},
    {aiTextureType_LIGHTMAP, "texture_lightmap"},
    {aiTextureType_REFLECTION, "texture_reflection"},
    {aiTextureType_BASE_COLOR, "texture_base_color"},
    {aiTextureType_NORMAL_CAMERA, "texture_normal_camera"},
    {aiTextureType_EMISSION_COLOR, "texture_emission_color"},
    {aiTextureType_METALNESS, "texture_metalness"},
    {aiTextureType_DIFFUSE_ROUGHNESS, "texture_diffuse_roughness"},
    {aiTextureType_AMBIENT_OCCLUSION, "texture_occlusion"},
#ifdef _WIN64
    {aiTextureType_CLEARCOAT, "texture_clearcoat"},
    {aiTextureType_TRANSMISSION, "texture_transmission"},
    {aiTextureType_SHEEN, "texture_sheen"},
    {aiTextureType_MAYA_BASE, "texture_maya_base"},
    {aiTextureType_MAYA_SPECULAR, "texture_maya_specular"},
    {aiTextureType_MAYA_SPECULAR_COLOR, "texture_maya_specular_color"},
    {aiTextureType_MAYA_SPECULAR_ROUGHNESS, "texture_maya_specular_roughness"},
    {aiTextureType_ANISOTROPY, "texture_anisotropy"},
    {aiTextureType_GLTF_METALLIC_ROUGHNESS, "texture_gltf_metallic_roughness"},
#endif
};

void debug_assimp_vertex_bones(const struct aiScene *scene);

void assimp__internal_glmesh_processMaterial(glmodel_t *self, const struct aiScene *scene, const u32 material_index) 
{
    const struct aiMaterial *material = scene->mMaterials[material_index];
    ASSERT(material);

    struct aiColor4D color;
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
        list_append(&self->colors, color);
    }

    //TODO: setup materials
}

glmesh_t assimp__internal_glmesh_processMesh(const struct aiMesh *mesh) {
    // get the total total indicies in the mesh
    u64 total_indicies = 0;
    for (u32 i = 0; i < mesh->mNumFaces; i++)
        total_indicies += mesh->mFaces[i].mNumIndices;

    slot_t vtx = slot_init(mesh->mNumVertices, sizeof(glvertex3d_t), NULL);
    slot_t ind = slot_init(total_indicies, sizeof(u32), NULL);

    // Copy indices
    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        struct aiFace face = mesh->mFaces[i];
        slot_insert_multiple(
            &ind, 
            (const u8 *)face.mIndices, 
            face.mNumIndices,
            sizeof(u32));
    }

    // Copy vertices, normals, UVs
    for (u32 i = 0; i < mesh->mNumVertices; i++) {
        ASSERT(mesh->mVertices && "Given mesh has no vertices");

        glvertex3d_t vt = {0};

        vt.pos = (vec3f_t){
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z,
        };

        if (mesh->mTextureCoords[0]) {
            //NOTE: this only loads primary uv texture 
            vt.uv = (vec2f_t){
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y,
            };
            vt.tangents = (vec3f_t){
                .x = mesh->mTangents[i].x,
                .y = mesh->mTangents[i].y,
                .z = mesh->mTangents[i].z,
            };
            vt.bitangents = (vec3f_t){
                .x = mesh->mBitangents[i].x,
                .y = mesh->mBitangents[i].y,
                .z = mesh->mBitangents[i].z,
            };
        }

        if (mesh->mNormals) {
            vt.norm = glms_normalize((vec3f_t){
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z,
            });
        }

        vt.bone_ids = (vec4i_t ){ -1, -1, -1, -1 }; 
        vt.bone_weights = (vec4f_t){ 0.0f, 0.0f, 0.0f, 0.0f };

        slot_append(&vtx, vt);
    }

    return (glmesh_t){
        .vtx = vtx, 
        .idx = ind,
        .material_index = mesh->mMaterialIndex
    };
}

i32 assimp__internal_get_bone_id(hashtable_t *bone_name_to_index, const struct aiBone *bone)
{
    i32 bone_id = 0;
    str_t bone_name = str__from_cstr(bone->mName.data, bone->mName.length);

    if (hashtable_has_key(bone_name_to_index, (hashtable_key_t){ .str = bone_name })) {
        bone_id = hashtable_get_value(bone_name_to_index, (hashtable_key_t){ .str = bone_name });
    } else {
        bone_id = bone_name_to_index->entries.len; //Sets total occupied entries as the index
        hashtable_insert(bone_name_to_index, (hashtable_key_t){bone_name}, bone_id);
    }
    return bone_id;
}

void assimp__internal_glmesh_process_bones(struct aiMesh *mesh, slot_t *vertices, hashtable_t *bone_name_to_index, list_t *bone_infos)
{
    //Parse individual bone
    for (u32 i = 0; i < mesh->mNumBones; i++)
    {
        const struct aiBone *bone = mesh->mBones[i];
        i32 bone_id = assimp__internal_get_bone_id(bone_name_to_index, bone);

        if (bone_id == (i32)bone_infos->len) {
            const boneinfo_t data = boneinfo(glms_mat4_transpose(*(matrix4f_t *)&bone->mOffsetMatrix));
            list_append(bone_infos, data);
        }

        for (u32 weight_count = 0; weight_count < bone->mNumWeights; weight_count++)
        {
            const struct aiVertexWeight vw = bone->mWeights[weight_count];

            //Update glvertex3d with vertex bone data
            glvertex3d_t *vtx = (glvertex3d_t *)slot_get_value(vertices, vw.mVertexId);
            bool found_entry = false;
            for (u32 bone_cmp = 0; bone_cmp < 4; ++bone_cmp) 
            {
                if (vtx->bone_ids.raw[bone_cmp] == -1) {
                    vtx->bone_ids.raw[bone_cmp] = bone_id;
                    vtx->bone_weights.raw[bone_cmp] = vw.mWeight;
                    found_entry = true;
                    break;
                }
            }

            if (!found_entry) {
                eprint("Serious bug - bone count per vertex exceeded here, check model whether each vertex has less than or equal to 4 bones");
            }
        }
    }
}

void print_u32(void *data)
{
    printf("%i\n", *(u32 *)data);
}

void assimp__internal_glmodel_read_node_heirarchy(const hashtable_t *bone_name_to_index, list_t *bone_info, const struct aiNode *node, const matrix4f_t parentTransform)
{
    const str_t node_name = str__from_cstr(node->mName.data, node->mName.length);
    const matrix4f_t node_transform = glms_mat4_transpose(*(matrix4f_t *)&node->mTransformation);
    const matrix4f_t global_transform = matrix4f_multiply(parentTransform, node_transform);

    if (hashtable_has_key(bone_name_to_index, (hashtable_key_t){node_name})) {
        const u32 bone_index = hashtable_get_value(bone_name_to_index, (hashtable_key_t){node_name});
        boneinfo_t *boneinfo = (boneinfo_t *)list_get_value(bone_info, bone_index);
        boneinfo->transform = matrix4f_multiply(global_transform, boneinfo->offset);
    }

    for (u32 i = 0; i < node->mNumChildren; i++)
    {
        assimp__internal_glmodel_read_node_heirarchy(bone_name_to_index, bone_info, node->mChildren[i], global_transform);
    }

}

void assimp__internal_glmodel_set_bone_transforms(glmodel_t *self, const hashtable_t *bone_name_to_index, const struct aiNode *node, const u32 mesh_index)
{
    assimp__internal_glmodel_read_node_heirarchy(
        bone_name_to_index, 
        &self->bone_infos, 
        node, 
        MATRIX4F_IDENTITY);

    list_iterator(&self->bone_infos, iter)
    {
        boneinfo_t *info = iter;
        list_append(&self->transforms[mesh_index], info->transform);
    }

    ASSERT(self->transforms[mesh_index].len <= MAX_BONES);
    ASSERT(self->transforms[mesh_index].len <= bone_name_to_index->entries.len);
}

u32 assimp__internal_get_total_bones(const struct aiScene *scene)
{
    u32 total = 0;
    for (u32 i = 0; i < scene->mNumMeshes; i++)
    {
        total += scene->mMeshes[i]->mNumBones;
    }
    return total;
}

void assimp__internal_glmesh_processScene(glmodel_t *self, const struct aiScene *scene) 
{

    //NOTE: Since assimp has mesh bones hold the local vertex id, and since we club together all vertices 
    //in a buffer, we need to translate vertex ids to bone ids - since its the reverse assimp gives us

    //Map all bones to an index for easy lookup
    const u32 total_bones = assimp__internal_get_total_bones(scene);
    if(total_bones) {
        self->bone_name_to_index = arena_reserve(&self->arena, sizeof(hashtable_t));
        *self->bone_name_to_index = hashtable_init(total_bones, HT_KEY_TYPE_STR, i32, &self->arena);
    }

    ASSERT(scene->mNumMeshes <= MAX_MESHES_PER_MODEL && "Update the transforms list in glmodel_t");
    for (u32 mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++)
    {
        struct aiMesh *mesh = scene->mMeshes[mesh_index];

        self->transforms[mesh_index] = list_init(matrix4f_t);

        // Process mesh
        glmesh_t m = assimp__internal_glmesh_processMesh(mesh);

        // Process materials
        assimp__internal_glmesh_processMaterial(
            self, 
            scene, 
            mesh->mMaterialIndex
        );

        if (mesh->mNumBones) {
            assimp__internal_glmesh_process_bones(mesh, &m.vtx, self->bone_name_to_index, &self->bone_infos);
            assimp__internal_glmodel_set_bone_transforms(
                self, 
                self->bone_name_to_index, 
                scene->mRootNode,
                mesh_index
            );
        }

        list_append(&self->meshes, m);
    }
}

void assimp__internal_glmodel_load_alltextures(glmodel_t *self, const struct aiScene *scene)
{
    for(u32 texture_index = 0; texture_index < scene->mNumTextures; texture_index++)
    {
        gltexture2d_t texture = {0};
        struct aiTexture *aitexture = scene->mTextures[texture_index];

        if (aitexture->mFilename.length == 0 && aitexture->mHeight == 0) {
            texture = gltexture2d_embedded_init((u8 *)aitexture->pcData, aitexture->mWidth);
        } else {
            str_t absolute_texture_path = str_join(&self->arena, &self->directory_path, aitexture->mFilename.data);
            texture = gltexture2d_init(absolute_texture_path.data);
        }
        list_append(&self->textures, texture);
    }
}


glmodel_t glmodel_init(const char *filepath) 
{
    glmodel_t o = {0};
    ASSERT(strlen(filepath) < ARRAY_LEN(o.filepath));
    memcpy(o.filepath, filepath, strlen(filepath));
    o.directory_path = str_get_directory_path(filepath);
    o.meshes = list_init(glmesh_t);
    o.textures = list_init(gltexture2d_t);
    o.colors = list_init(vec4f_t);
    o.bone_infos = list_init(boneinfo_t);
    o.animator = animator_init();
    o.current_time = 0.0f;
    o.arena = arena_init(NULL, 2 * MB);

    logging("Loading model %s ...", filepath);

    // Assimp: import model
    const struct aiScene *scene = aiImportFile(
        filepath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices); // http://assimp.sourceforge.net/lib_html/structai_scene.html
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        eprint("ERROR::ASSIMP:: %s", aiGetErrorString());
    }

    o.scene = scene;

    //NOTE: This is used to get the vertex in world space back to model space
    o.global_inverse_transform  = glms_mat4_inv(
        glms_mat4_transpose(
            *(matrix4f_t *)&scene->mRootNode->mTransformation
        )
    );

    assimp__internal_glmodel_load_alltextures(&o, scene);

    //debug_assimp_vertex_bones(scene);
    assimp__internal_glmesh_processScene(&o, scene);

    //load all animations
    animator_load_all_animations(&o.animator, scene);

    logging("Completed loading model %s ...", filepath);

    return o;
}

void glmodel_destroy(glmodel_t *self) 
{
    for (u32 i = 0; i < self->meshes.len; i++) 
        list_destroy(&self->transforms[i]);

    list_iterator(&self->meshes, iter) { 
        glmesh_destroy((glmesh_t *)iter); 
    }
    list_destroy(&self->meshes);

    list_iterator(&self->textures, iter) { gltexture2d_destroy(iter); }
    list_destroy(&self->textures);

    list_destroy(&self->colors);

    list_destroy(&self->bone_infos);

    animator_destroy(&self->animator);

    aiReleaseImport(self->scene);
    arena_destroy(&self->arena);

    memset(self->filepath, 0, sizeof(self->filepath));
}

void debug_assimp_vertex_bones(const struct aiScene *scene) {
    if (!scene || !scene->mMeshes) {
        printf("Error: Invalid or empty scene.\n");
        return;
    }

    printf("Debugging vertex bone assignments for scene\n");
    printf("Total meshes: %u\n", scene->mNumMeshes);
    printf("Total bones across all meshes: ");
    u32 total_bones = 0;
    for (u32 i = 0; i < scene->mNumMeshes; i++) {
        total_bones += scene->mMeshes[i]->mNumBones;
    }
    printf("%u\n", total_bones);

    // Iterate through each mesh
    for (u32 mesh_idx = 0; mesh_idx < scene->mNumMeshes; mesh_idx++) {
        struct aiMesh *mesh = scene->mMeshes[mesh_idx];
        printf("\nMesh %u: %s\n", mesh_idx, mesh->mName.data);
        printf("  Total vertices: %u\n", mesh->mNumVertices);
        printf("  Total bones: %u\n", mesh->mNumBones);

        // Create an array to track bone weights for each vertex
        struct VertexBoneInfo {
            u32 bone_ids[4];
            float weights[4];
            u32 count;
        } *vertex_bones = calloc(mesh->mNumVertices, sizeof(struct VertexBoneInfo));
        if (!vertex_bones) {
            printf("Error: Failed to allocate memory for vertex bone info.\n");
            continue;
        }

        // Process each bone and its weights
        for (u32 bone_idx = 0; bone_idx < mesh->mNumBones; bone_idx++) {
            struct aiBone *bone = mesh->mBones[bone_idx];
            for (u32 weight_idx = 0; weight_idx < bone->mNumWeights; weight_idx++) {
                struct aiVertexWeight *weight = &bone->mWeights[weight_idx];
                u32 vertex_id = weight->mVertexId;
                float weight_value = weight->mWeight;

                // Add bone influence to the vertex
                if (vertex_bones[vertex_id].count < 4) {
                    vertex_bones[vertex_id].bone_ids[vertex_bones[vertex_id].count] = bone_idx;
                    vertex_bones[vertex_id].weights[vertex_bones[vertex_id].count] = weight_value;
                    vertex_bones[vertex_id].count++;
                } else {
                    printf("Warning: Vertex %u in mesh %u has more than 4 bones affecting it.\n", vertex_id, mesh_idx);
                }
            }
        }

        // Print bone information for each vertex
        for (u32 vertex_id = 0; vertex_id < mesh->mNumVertices; vertex_id++) {
            printf("  Vertex %u (pos: %.2f, %.2f, %.2f):\n", vertex_id,
                   mesh->mVertices[vertex_id].x, mesh->mVertices[vertex_id].y, mesh->mVertices[vertex_id].z);
            printf("    Bones affecting vertex: %u\n", vertex_bones[vertex_id].count);
            for (u32 i = 0; i < vertex_bones[vertex_id].count; i++) {
                printf("      Bone ID: %u (Name: %s), Weight: %.4f\n",
                       vertex_bones[vertex_id].bone_ids[i],
                       mesh->mBones[vertex_bones[vertex_id].bone_ids[i]]->mName.data,
                       vertex_bones[vertex_id].weights[i]);
            }
        }

        free(vertex_bones);
    }
    printf("\n");
}

// Helper function to process node hierarchy for animation
void assimp__internal_process_node_anim(glmodel_t *self, struct aiNode *node, const matrix4f_t parent_transform, const list_t *channels, animation_t *current_anim) {
    ASSERT(node);

    const str_t node_name = str__from_cstr(node->mName.data, node->mName.length);
    matrix4f_t node_transform = glms_mat4_transpose(*(matrix4f_t *)&node->mTransformation);

    // Find animation channel for this node
    list_iterator(channels, iter) {
        node_anim_t *channel = iter;
        if (strcmp(channel->node_name, node_name.data) == 0) {
            node_transform = compute_node_transform(channel, self->current_time, current_anim->duration);
            break;
        }
    }

    // Compute global transform
    const matrix4f_t global_transform = matrix4f_multiply(parent_transform, node_transform);

    // Update bone transform if this node is a bone
    if (hashtable_has_key(self->bone_name_to_index, (hashtable_key_t){node_name})) {
        u32 bone_index = hashtable_get_value(self->bone_name_to_index, (hashtable_key_t){node_name});
        boneinfo_t *bone_info = (boneinfo_t *)list_get_value(&self->bone_infos, bone_index);
        bone_info->transform = 
            matrix4f_multiply(
                self->global_inverse_transform,
                matrix4f_multiply(global_transform, bone_info->offset)
            );

        // Assign to transforms array for each mesh
        for (u32 mesh_idx = 0; mesh_idx < self->meshes.len; mesh_idx++) {
            list_append(&self->transforms[mesh_idx], bone_info->transform);
        }
    }

    // Recurse through children
    for (u32 i = 0; i < node->mNumChildren; i++) {
        assimp__internal_process_node_anim(self, node->mChildren[i], global_transform, channels, current_anim);
    }
}

void glmodel_set_animation(glmodel_t *self, const char *animation_label, const f32 dt, const bool loop)
{
    if (self->animator.animations.len == 0) {
        logging("No animations to process");
        return;
    }

    // Get the specified animation
    animation_t *current_anim = animator_get_animation(&self->animator, animation_label);
    if (!current_anim || current_anim->ticks_per_second == 0.0f) {
        logging("Invalid animation or ticks per second is zero");
        return;
    }

    // Track current animation and time to reset time on animation change
    static const char *last_animation = NULL;
    if (last_animation != animation_label) {
        self->current_time = 0.0f; // Reset time when switching animations
        last_animation = animation_label;
    }

    // Increment animation time (convert dt from seconds to ticks)
    self->current_time += dt * current_anim->ticks_per_second;
    if (loop) {
        self->current_time = fmod(self->current_time, current_anim->duration); // Loop animation
    }

    // Clear previous transforms
    for (u32 i = 0; i < self->meshes.len; i++) {
        list_clear(&self->transforms[i]);
    }

    // Traverse node hierarchy starting from root
    if (self->scene->mRootNode) {
        assimp__internal_process_node_anim(self, self->scene->mRootNode, MATRIX4F_IDENTITY, &current_anim->channels, current_anim);
    } else {
        eprint("No root node available.");
    }

    // Pad transforms to MAX_BONES
    for (u32 mesh_idx = 0; mesh_idx < self->meshes.len; mesh_idx++) {
        while (self->transforms[mesh_idx].len < MAX_BONES) {
            list_append(&self->transforms[mesh_idx], MATRIX4F_IDENTITY);
        }
    }
}

gltexturelist_t glmodel_get_texuturelist(const glmodel_t *self)
{
    if (!self->textures.len) return (gltexturelist_t){0};
    ASSERT(self->textures.len <= MAX_SUPPORTED_TEXTURE_COUNT_PER_DRAW_CALL);

    gltexturelist_t list = {
        .count = self->textures.len,
        .items = {0}
    };
    list_iterator(&self->textures, iter) {
        list.items[(u64)list_index] = (gltextureitem_t ){
            .type = GL_TEXTURE_TYPE_NORMAL,
            .source = iter
        };
    }
    return list;
}

#endif

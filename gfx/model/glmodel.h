#pragma once
#include <poglib/math.h>
#include <poglib/external/extensions/cgltf.h>

//NOTE: Fixes for issues found in playing animation 
//* This system expects all animations to be baked i.e node->has_matrix is 0 always, by default blenders exporters does this unless set otherwise

typedef struct {

    u16     current_anim_idx;
    u16     target_anim_idx;
    f32     current_time;
    f32     target_time;
    f32     blend_duration;
    bool    is_transitioning;

} anim_state_machine_t;

typedef struct {
    vec3s   translation;
    versors rotation;
    vec3s   scale;
} glmodel_node_transform_state_t;

typedef struct {
    cgltf_data *data;
    anim_state_machine_t anim_state_machine;
    mat4s *world_matrices;
    mat4s *uJoints_palette;
} glmodel_t;

typedef struct {
    str_t   name;
    f32     duration;
} animation_t;

glmodel_t   glmodel_load_model_init(const str_t filepath, arena_t *const arena);
glmodel_t   glmodel_load_level_from_file_init(const str_t filepath, arena_t *const arena);

bool        glmodel_is_animation_playing(const glmodel_t *const self, const u32 anim_idx);
f32         glmodel_get_playing_animation_loop_count(glmodel_t *const self, const f32 dt);
f32         glmodel_get_animation_duration(const glmodel_t *const self, const u16 idx);
u32         glmodel_get_animation_idx_from_label(const glmodel_t *const self, const str_t animation_label);

void        glmodel_play_animation(glmodel_t *const self, const u32 new_anim_idx, const f32 blendspeed);
void        glmodel_update(glmodel_t *const self, const f32 dt);

void        glmodel_destroy(glmodel_t *const self);


f32 glmodel_get_animation_duration(const glmodel_t *const self, const u16 idx)
{
    if (!self || !self->data || idx >= self->data->animations_count) return 0.0f;

    const cgltf_animation *anim = &self->data->animations[idx];
    f32 max_time = 0.0f;

    for (cgltf_size samp_idx = 0; samp_idx < anim->samplers_count; ++samp_idx)
    {
        const cgltf_accessor *input = anim->samplers[samp_idx].input;
        if (!input || input->count == 0) continue;

        f32 sampler_max = 0.0f;
        if (input->has_max) {
            sampler_max = input->max[0];
        } else {
            cgltf_accessor_read_float(input, input->count - 1, &sampler_max, 1);
        }

        if (sampler_max > max_time) {
            max_time = sampler_max;
        }
    }

    return max_time;
}

INTERNAL u16 glmodel__internal__playing_anim_idx(const glmodel_t *const self)
{
    return self->anim_state_machine.is_transitioning
        ? self->anim_state_machine.target_anim_idx
        : self->anim_state_machine.current_anim_idx;
}

f32 glmodel_get_playing_animation_loop_count(glmodel_t *const self, const f32 dt)
{
    ASSERT(self && self->data);

    const anim_state_machine_t *anim_sm = &self->anim_state_machine;
    const u16 playing_idx               = glmodel__internal__playing_anim_idx(self);
    const f32 duration                  = glmodel_get_animation_duration(self, playing_idx);

    if (duration <= 0.001f)
        eprint("Recheck animation `%s` \'s duration, found value as `%f`. System expects a bigger value.", self->data->animations[playing_idx].name, duration);

    const f32 playing_time = anim_sm->is_transitioning 
        ? anim_sm->target_time
        : anim_sm->current_time;

    const f32 loop_count = (playing_time + dt) / duration;
    return loop_count;
}

glmodel_t glmodel_load_model_init(const str_t filepath, arena_t *const arena)
{
    ASSERT(arena);

    cgltf_data *data_buffer     = NULL;
    const cgltf_options options = {0};
    file_t file = file_init(filepath.data, "rb");
    {
        ASSERT(file.size);

        u8 *buffer = arena_reserve(arena, file.size);
        {
            file_readbytes(&file, buffer, file.size);
            const cgltf_result result = cgltf_parse(&options, buffer, file.size, &data_buffer);
            if (result != cgltf_result_success) {
                eprint("unable to load "STR_FMT" | ERROR: %s", STR_ARG(filepath), CGLTF_RESULT_CSTR_LOOKUP[result]);
            }

            const cgltf_result load_result = cgltf_load_buffers(&options, data_buffer, filepath.data);
            if (load_result != cgltf_result_success) {
                eprint("failed to load buffers for "STR_FMT" | ERROR: %s", STR_ARG(filepath), CGLTF_RESULT_CSTR_LOOKUP[load_result]);
            }
        }
    }
    file_destroy(&file);

    cgltf_skin* skin        = (data_buffer->skins_count > 0) ? &data_buffer->skins[0] : NULL;
    mat4s* world_matrices   = skin ? (mat4s*)arena_reserve(arena, data_buffer->nodes_count * sizeof(mat4s)) : NULL;
    mat4s* uJoints_palette  = skin ? (mat4s*)arena_reserve(arena, skin->joints_count * sizeof(mat4s)) : NULL;

    if (uJoints_palette) {
        for (u32 idx = 0; idx < skin->joints_count; idx++) 
            uJoints_palette[idx] = glms_mat4_identity();
    }

    logging("Loaded model file `"STR_FMT"`", STR_ARG(filepath));

    return (glmodel_t) {
        .data = data_buffer,
        .anim_state_machine = (anim_state_machine_t){
            .current_anim_idx = 0,
            .target_anim_idx = 0,
            .blend_duration = 1.0f,
        },
        .world_matrices = world_matrices,
        .uJoints_palette = uJoints_palette,
    };
}

glmodel_t glmodel_load_level_from_file_init(const str_t filepath, arena_t *const arena)
{
    ASSERT(arena);

    cgltf_data *data_buffer     = NULL;
    const cgltf_options options = {0};
    file_t file = file_init(filepath.data, "rb");
    {
        ASSERT(file.size);

        u8 *buffer = arena_reserve(arena, file.size);
        {
            file_readbytes(&file, buffer, file.size);
            const cgltf_result result = cgltf_parse(&options, buffer, file.size, &data_buffer);
            if (result != cgltf_result_success) {
                eprint("unable to load "STR_FMT" | ERROR: %s", STR_ARG(filepath), CGLTF_RESULT_CSTR_LOOKUP[result]);
            }

            const cgltf_result load_result = cgltf_load_buffers(&options, data_buffer, filepath.data);
            if (load_result != cgltf_result_success) {
                eprint("failed to load buffers for "STR_FMT" | ERROR: %s", STR_ARG(filepath), CGLTF_RESULT_CSTR_LOOKUP[load_result]);
            }
        }
    }
    file_destroy(&file);


    logging("Loaded model file `"STR_FMT"`", STR_ARG(filepath));

    return (glmodel_t) {
        .data = data_buffer,
        .anim_state_machine = {0},
        .world_matrices = NULL,
        .uJoints_palette = NULL,
    };
}

void glmodel_destroy(glmodel_t *const self)
{
    ASSERT(self);
    ASSERT(self->data);

    //NOTE: keeping it here just incase we ever need it
}

INTERNAL u32 glmodel__internal__find_keyframe_index(const f32 *times, const u32 count, const f32 t)
{
    for (u32 i = 0; i < count - 1; ++i) {
        if (t >= times[i] && t < times[i + 1]) return i;
    }
    return count - 2;
}

INTERNAL mat4s glmodel__internal__compute_node_local_matrix_s(const cgltf_node* node) 
{
    if (node->has_matrix) {
        return glms_mat4_ucopy(*(mat4s *)&node->matrix);
    }

    mat4s local_val = glms_mat4_identity();

    if (node->has_translation) {
        vec3s t = { node->translation[0], node->translation[1], node->translation[2] };
        local_val = glms_translate(local_val, t);
    }

    if (node->has_rotation) {
        versors q = { node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] };
        mat4s rot_mat = glms_quat_mat4(q);
        local_val = glms_mat4_mul(local_val, rot_mat);
    }

    if (node->has_scale) {
        vec3s s = { node->scale[0], node->scale[1], node->scale[2] };
        local_val = glms_scale(local_val, s);
    }

    return local_val;
}

INTERNAL void glmodel__internal__compute_node_world_recursive(
    const cgltf_node* node, 
    mat4s parent_world, 
    const cgltf_data* data, 
    mat4s* out_world_matrices
) {
    const u32 node_idx = (u32)(node - data->nodes);

    const mat4s local_mat = glmodel__internal__compute_node_local_matrix_s(node);
    const mat4s world_mat = glms_mat4_mul(parent_world, local_mat);
    out_world_matrices[node_idx] = world_mat;

    for (u32 i = 0; i < node->children_count; ++i) {
        glmodel__internal__compute_node_world_recursive(node->children[i], world_mat, data, out_world_matrices);
    }
}

INTERNAL void glmodel__internal__compute_world_matrices_s(const cgltf_data* data, mat4s* out_world_matrices) 
{
    const cgltf_scene* scene = data->scene ? data->scene : &data->scenes[0];
    mat4s identity = glms_mat4_identity();

    for (u32 i = 0; i < scene->nodes_count; ++i) {
        glmodel__internal__compute_node_world_recursive(scene->nodes[i], identity, data, out_world_matrices);
    }
}

INTERNAL void glmodel__internal__compute_joint_matrices_s(
    const cgltf_data* data,
    const cgltf_skin* skin,
    const mat4s* world_matrices,
    mat4s* out_joint_matrices
) {
    if (!skin || !skin->inverse_bind_matrices) return;

    cgltf_accessor* ibm_accessor = skin->inverse_bind_matrices;

    for (u32 i = 0; i < skin->joints_count; ++i) 
    {
        cgltf_node* joint_node = skin->joints[i];

        uint32_t node_idx = (uint32_t)(joint_node - data->nodes);
        mat4s world_mat = world_matrices[node_idx];

        mat4s ibm_mat;
        cgltf_accessor_read_float(ibm_accessor, i, (float*)ibm_mat.raw, 16);

        out_joint_matrices[i] = glms_mat4_mul(world_mat, ibm_mat);
    }
}

INTERNAL void glmodel__internal__sample_animation_pose(
    const cgltf_animation *anim, 
    const float time, 
    const cgltf_data *data,
    glmodel_node_transform_state_t *out_pose
) {
    for (u32 node_idx = 0; node_idx < data->nodes_count; ++node_idx)
    {
        cgltf_node* node = &data->nodes[node_idx];
        out_pose[node_idx].translation = node->has_translation 
            ? (vec3s){node->translation[0], node->translation[1], node->translation[2]} 
            : glms_vec3_zero();

        out_pose[node_idx].rotation = node->has_rotation 
            ? (versors){node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]} 
            : glms_quat_identity();

        out_pose[node_idx].scale = node->has_scale 
            ? (vec3s){node->scale[0], node->scale[1], node->scale[2]} 
            : glms_vec3_one();
    }

    for (u32 channel_idx = 0; channel_idx < anim->channels_count; ++channel_idx)
    {
        const cgltf_animation_channel* channel = &anim->channels[channel_idx];
        const cgltf_animation_sampler* sampler = channel->sampler;
        cgltf_node* target = channel->target_node;

        if (!target || !sampler->input || !sampler->output) continue;

        const u32 node_idx = (u32)(target - data->nodes);
        const u32 keyframe_count = sampler->input->count;
        if (keyframe_count < 2) continue;

        f32 *times = (f32*)stackarena_push(global_runtimectx->stackarena, keyframe_count * sizeof(f32));
        cgltf_accessor_unpack_floats(sampler->input, times, keyframe_count);

        const f32 clamped_time = glm_clamp(time, times[0], times[keyframe_count - 1]);
        const u32 k = glmodel__internal__find_keyframe_index(times, keyframe_count, clamped_time);

        const f32 t0 = times[k];
        const f32 t1 = times[k + 1];
        const f32 factor = (t1 > t0) ? ((clamped_time - t0) / (t1 - t0)) : 0.0f;

        stackarena_pop(global_runtimectx->stackarena);

        if (channel->target_path == cgltf_animation_path_type_translation) {

            vec3s v0, v1;
            cgltf_accessor_read_float(sampler->output, k,     v0.raw, 3);
            cgltf_accessor_read_float(sampler->output, k + 1, v1.raw, 3);
            out_pose[node_idx].translation = glms_vec3_lerp(v0, v1, factor);

        } else if (channel->target_path == cgltf_animation_path_type_rotation) {

            versors q0, q1;
            cgltf_accessor_read_float(sampler->output, k,     q0.raw, 4);
            cgltf_accessor_read_float(sampler->output, k + 1, q1.raw, 4);
            out_pose[node_idx].rotation = glms_quat_slerp(q0, q1, factor);

        } else if (channel->target_path == cgltf_animation_path_type_scale) {

            vec3s s0, s1;
            cgltf_accessor_read_float(sampler->output, k,     s0.raw, 3);
            cgltf_accessor_read_float(sampler->output, k + 1, s1.raw, 3);
            out_pose[node_idx].scale = glms_vec3_lerp(s0, s1, factor);

        }
    }
}

INTERNAL void glmodel__internal__blend_poses(
    glmodel_node_transform_state_t *pose_a, 
    const glmodel_node_transform_state_t *pose_b, 
    const f32 alpha, 
    const u32 node_count
) {
    const f32 cl_alpha = glm_clamp(alpha, 0.0f, 1.0f);

    for (u32 i = 0; i < node_count; ++i) 
    {
        pose_a[i].translation = glms_vec3_lerp(pose_a[i].translation, pose_b[i].translation, cl_alpha);
        pose_a[i].rotation    = glms_quat_slerp(pose_a[i].rotation, pose_b[i].rotation, cl_alpha);
        pose_a[i].scale       = glms_vec3_lerp(pose_a[i].scale, pose_b[i].scale, cl_alpha);
    }
}

void glmodel_play_animation(glmodel_t *const self, const u32 new_anim_idx, const f32 blendspeed)
{
    ASSERT(self && self->data);
    if (new_anim_idx >= self->data->animations_count) return;

    anim_state_machine_t *const anim_sm = &self->anim_state_machine;

    if ((anim_sm->is_transitioning && (anim_sm->target_anim_idx == new_anim_idx)) || (anim_sm->current_anim_idx == new_anim_idx))
        return;

    const bool instant = blendspeed >= 1.0f;
    if (instant)
    {
        anim_sm->current_anim_idx = new_anim_idx;
        anim_sm->target_anim_idx  = new_anim_idx;
        anim_sm->current_time     = 0.0f;
        anim_sm->target_time      = 0.0f;
        anim_sm->is_transitioning = false;
        return;
    }

    if (anim_sm->is_transitioning)
    {
        anim_sm->current_anim_idx = anim_sm->target_anim_idx;
        anim_sm->current_time     = anim_sm->target_time;
    }

    anim_sm->target_anim_idx  = new_anim_idx;
    anim_sm->target_time      = 0.0f;
    anim_sm->blend_duration   = (blendspeed > 0.0f && blendspeed < 1.0f) ? blendspeed : 1.0f;
    anim_sm->is_transitioning = true;
}

void glmodel_update(glmodel_t *const self, const f32 dt)
{
    ASSERT(self && self->data);

    anim_state_machine_t *anim_sm = &self->anim_state_machine;

    cgltf_data* data = self->data;
    if (!data->animations_count) return;

    const cgltf_skin* skin = (data->skins_count > 0) 
        ? &data->skins[0] 
        : NULL;

    for (u32 idx = 0; idx < data->nodes_count; idx++)               self->world_matrices[idx]  = glms_mat4_identity();
    for (u32 idx = 0; (skin && idx < skin->joints_count); idx++)    self->uJoints_palette[idx] = glms_mat4_identity();

    glmodel_node_transform_state_t *pose_a = (glmodel_node_transform_state_t *)stackarena_push(global_runtimectx->stackarena, sizeof(glmodel_node_transform_state_t) * data->nodes_count);
    glmodel_node_transform_state_t *pose_b = (glmodel_node_transform_state_t *)stackarena_push(global_runtimectx->stackarena, sizeof(glmodel_node_transform_state_t) * data->nodes_count);

    anim_sm->current_time += dt;

    const cgltf_animation* anim_a = &data->animations[anim_sm->current_anim_idx];
    const f32 duration_a = glmodel_get_animation_duration(self, anim_sm->current_anim_idx);
    ASSERT(duration_a > 0.0f);

    const f32 sampled_time_a = !anim_sm->is_transitioning
        ? fmodf(anim_sm->current_time, duration_a) 
        : anim_sm->current_time;

    glmodel__internal__sample_animation_pose(anim_a, sampled_time_a, data, pose_a);

    if (anim_sm->is_transitioning) {

        anim_sm->target_time += dt;

        const f32 duration_b        = glmodel_get_animation_duration(self, anim_sm->target_anim_idx);
        const f32 sampled_time_b    = (duration_b > 0.0f) ? fmodf(anim_sm->target_time, duration_b) : anim_sm->target_time;

        const f32 alpha = (anim_sm->blend_duration > 0.0f)
            ? (anim_sm->target_time / anim_sm->blend_duration)
            : 1.0f;

        const cgltf_animation* anim_b = &data->animations[anim_sm->target_anim_idx];

        glmodel__internal__sample_animation_pose(anim_b, sampled_time_b, data, pose_b);
        glmodel__internal__blend_poses(pose_a, pose_b, alpha, data->nodes_count);

        if (alpha >= 1.0f) {
            anim_sm->current_anim_idx = anim_sm->target_anim_idx;
            anim_sm->current_time     = anim_sm->target_time;
            anim_sm->target_time      = 0.0f;
            anim_sm->is_transitioning = false;
        }
    }

    for (u32 i = 0; i < data->nodes_count; ++i) 
    {
        cgltf_node* node = &data->nodes[i];
        node->has_matrix = 0;
        node->has_translation = 1;
        node->has_rotation = 1;
        node->has_scale = 1;

#if defined(_WIN64)
        glm_vec3_copy(pose_a[i].translation.raw, node->translation);
        glm_vec4_copy(pose_a[i].rotation.raw, node->rotation);
        glm_vec3_copy(pose_a[i].scale.raw, node->scale);
#else
        //TODO: maybe move to above version after testing in linux
        *(vec3s *)node->translation = pose_a[i].translation;
        *(versors *)node->rotation  = pose_a[i].rotation; 
        *(vec3s *)node->scale       = pose_a[i].scale;
#endif
    }

    stackarena_pop(global_runtimectx->stackarena);
    stackarena_pop(global_runtimectx->stackarena);

    glmodel__internal__compute_world_matrices_s(data, self->world_matrices);
    if (skin && self->uJoints_palette) {
        glmodel__internal__compute_joint_matrices_s(data, skin, self->world_matrices, self->uJoints_palette);
    }
}


u32 glmodel_get_animation_idx_from_label(const glmodel_t *const self, const str_t animation_label)
{
    for (u32 idx = 0; idx < self->data->animations_count; idx++)
    {
        if (strcmp(animation_label.data, self->data->animations[idx].name) == 0)
            return idx;
    }
    eprint("animation label `"STR_FMT"` does not exist", animation_label);
    return 0;
}

bool glmodel_is_animation_playing(const glmodel_t *const self, const u32 anim_idx)
{
    return self->anim_state_machine.current_anim_idx == anim_idx
        || self->anim_state_machine.target_anim_idx == anim_idx;
}


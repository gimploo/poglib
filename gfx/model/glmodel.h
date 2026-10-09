#pragma once
#include <poglib/math.h>
#include <poglib/external/extensions/cgltf.h>

//TODO: the animation is heavily AI assited, breaking down the animation interface cgltf would
//be a good start to bringing the ASSIMP functionality back working.
//First would be to get rid of the deprecated functions that were brought in to support the
//existing parkour system - possibly there could be a better alternative to the old system using 
//loop count maybe keeping the 
//
//so much of this is just slop, improve later


//TODO: the animation has started to loop now, but doesnt switch to other one although the 
//state is switched - need to investigate.

typedef struct {
    u16 current_anim_idx;
    u16 target_anim_idx;
    float current_time;
    float target_time;
    float transition_time;     // Time elapsed in transition
    float transition_duration; // Total transition length (e.g., 0.25s)
    bool is_transitioning;
} anim_state_machine_t;

typedef struct {
    vec3s translation;
    versors rotation;
    vec3s scale;
} glmodel_TRS_state_t;

typedef struct {
    cgltf_data *data;
    anim_state_machine_t anim_state_machine;
    mat4s *world_matrices;
    mat4s *uJoints_palette;
} glmodel_t;

glmodel_t   glmodel_load_model_init(const str_t filepath, arena_t *const arena);
glmodel_t   glmodel_load_level_from_file_init(const str_t filepath, arena_t *const arena);
void        glmodel_update(glmodel_t *self, f32 dt);
void        glmodel_play_animation(glmodel_t *self, const u16 new_anim_idx);
void        glmodel_destroy(glmodel_t *const self);

typedef struct {
    char *name;
    f32 ticks_per_second;
    f32 duration;
} animation_t;

// -----------------------------------------------------------------------------
// Helper: Fetch total animation duration by checking keyframe timestamps
// -----------------------------------------------------------------------------
f32 glmodel_get_animation_duration(const glmodel_t *const self, const u16 idx)
{
    if (!self || !self->data || idx >= self->data->animations_count) return 0.0f;

    cgltf_animation anim = self->data->animations[idx];
    f32 max_time = 0.0f;

    for (cgltf_size i = 0; i < anim.samplers_count; ++i) {
        cgltf_accessor* input = anim.samplers[i].input;

        if (input->has_max) {
            if (input->max[0] > max_time) {
                max_time = input->max[0];
            }
        } else {
            cgltf_size last_index = input->count - 1;
            f32 time = 0.0f;
            cgltf_accessor_read_float(input, last_index, &time, 1);
            if (time > max_time) {
                max_time = time;
            }
        }
    }
    return max_time;
}

// -----------------------------------------------------------------------------
// Restored Function: glmodel_get_playing_animation
// -----------------------------------------------------------------------------
animation_t glmodel_get_playing_animation(glmodel_t *self)
{
    ASSERT(self && self->data);

    u16 current_idx = self->anim_state_machine.current_anim_idx;
    cgltf_animation *anim = &self->data->animations[current_idx];

    return (animation_t) {
        .name = anim->name ? anim->name : "Unnamed",
        .duration = glmodel_get_animation_duration(self, current_idx),
        .ticks_per_second = 1.0f // cgltf timestamps are natively normalized in seconds
    };
}

// -----------------------------------------------------------------------------
// Restored Function: glmodel_set_animation
// -----------------------------------------------------------------------------
animation_t glmodel_set_animation(glmodel_t *self, const str_t animation_label, const f32 blendspeed)
{
    ASSERT(self && self->data);

    bool found = false;

    const char *current_anim = self->data->animations[self->anim_state_machine.current_anim_idx].name;
    char *target_anim = "";

    // Search for animation index by name matching
    u16 new_anim_idx = 0;
    for (u16 anim_idx = 0; anim_idx < self->data->animations_count; anim_idx++) {
        target_anim = self->data->animations[anim_idx].name;
        if (target_anim && str_cmp(animation_label, str_from_cstr((char *)target_anim, strlen(target_anim)))) {
            new_anim_idx = anim_idx;
            found = true;
            break;
        }
    }

    ASSERT(new_anim_idx != 0);

    logging("Animation transition %s -> "STR_FMT, current_anim, STR_ARG(animation_label));

    if (!found) {
        logging("Warning: Animation '%.*s' not found. Defaulting to index 0.", (int)animation_label.len, animation_label.data);
    }

    if (blendspeed > 0.0f) {
        self->anim_state_machine.transition_duration = blendspeed;
    } else {
        self->anim_state_machine.transition_duration = 1.0f;
    }

    glmodel_play_animation(self, new_anim_idx);

    return glmodel_get_playing_animation(self);
}

// -----------------------------------------------------------------------------
// Restored Function: glmodel_get_playing_animation_loop_count
// -----------------------------------------------------------------------------
f32 glmodel_get_playing_animation_loop_count(glmodel_t *self, const f32 dt)
{
    ASSERT(self && self->data);

    f32 duration = glmodel_get_animation_duration(self, self->anim_state_machine.current_anim_idx);
    if (duration <= 0.001f) return 0.0f;

    // Calculates fractional loop count (e.g. 2.5 loops elapsed)
    return self->anim_state_machine.current_time / duration;
}

glmodel_t glmodel_load_model_init(const str_t filepath, arena_t *const arena)
{
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

    logging("Loaded model file `"STR_FMT"`", STR_ARG(filepath));

    return (glmodel_t) {
        .data = data_buffer,
        .anim_state_machine = (anim_state_machine_t){
            .current_anim_idx = 0,
            .target_anim_idx = 0,
            .transition_duration = 1.0f // 250ms blend
        },
        .world_matrices = world_matrices,
        .uJoints_palette = uJoints_palette
    };
}

glmodel_t glmodel_load_level_from_file_init(const str_t filepath, arena_t *const arena)
{
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
        .uJoints_palette = NULL
    };
}

void glmodel_destroy(glmodel_t *const self)
{
    ASSERT(self);
    ASSERT(self->data);

    cgltf_free(self->data);
}

INTERNAL size_t glmodel__internal__find_keyframe_index(const float* times, size_t count, float t)
{
    for (size_t i = 0; i < count - 1; ++i) {
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
    uint32_t node_idx = (uint32_t)(node - data->nodes);

    mat4s local_mat = glmodel__internal__compute_node_local_matrix_s(node);
    mat4s world_mat = glms_mat4_mul(parent_world, local_mat);
    out_world_matrices[node_idx] = world_mat;

    for (size_t i = 0; i < node->children_count; ++i) {
        glmodel__internal__compute_node_world_recursive(node->children[i], world_mat, data, out_world_matrices);
    }
}

INTERNAL void glmodel__internal__compute_world_matrices_s(const cgltf_data* data, mat4s* out_world_matrices) 
{
    const cgltf_scene* scene = data->scene ? data->scene : &data->scenes[0];
    mat4s identity = glms_mat4_identity();

    for (size_t i = 0; i < scene->nodes_count; ++i) {
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

    for (size_t i = 0; i < skin->joints_count; ++i) {
        cgltf_node* joint_node = skin->joints[i];

        uint32_t node_idx = (uint32_t)(joint_node - data->nodes);
        mat4s world_mat = world_matrices[node_idx];

        mat4s ibm_mat;
        cgltf_accessor_read_float(ibm_accessor, i, (float*)ibm_mat.raw, 16);

        out_joint_matrices[i] = glms_mat4_mul(world_mat, ibm_mat);
    }
}

INTERNAL void glmodel__internal__sample_animation_pose(
    const cgltf_animation* anim, 
    float time, 
    const cgltf_data* data,
    glmodel_TRS_state_t *out_pose
) {
    for (size_t i = 0; i < data->nodes_count; ++i)
    {
        cgltf_node* node = &data->nodes[i];
        out_pose[i].translation = node->has_translation 
            ? (vec3s){node->translation[0], node->translation[1], node->translation[2]} 
            : glms_vec3_zero();

        out_pose[i].rotation = node->has_rotation 
            ? (versors){node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]} 
            : glms_quat_identity();

        out_pose[i].scale = node->has_scale 
            ? (vec3s){node->scale[0], node->scale[1], node->scale[2]} 
            : glms_vec3_one();
    }

    for (size_t i = 0; i < anim->channels_count; ++i)
    {
        const cgltf_animation_channel* channel = &anim->channels[i];
        const cgltf_animation_sampler* sampler = channel->sampler;
        cgltf_node* target = channel->target_node;

        if (!target || !sampler->input || !sampler->output) continue;

        uint32_t node_idx = (uint32_t)(target - data->nodes);
        size_t keyframe_count = sampler->input->count;
        if (keyframe_count < 2) continue;

        // Unpack time accessor safely on stack arena
        float* times = (f32*)stackarena_push(global_runtimectx->stackarena, keyframe_count * sizeof(f32));
        cgltf_accessor_unpack_floats(sampler->input, times, keyframe_count);

        float clamped_time = glm_clamp(time, times[0], times[keyframe_count - 1]);
        size_t k = glmodel__internal__find_keyframe_index(times, keyframe_count, clamped_time);

        float t0 = times[k];
        float t1 = times[k + 1];
        float factor = (t1 > t0) ? ((clamped_time - t0) / (t1 - t0)) : 0.0f;

        stackarena_pop(global_runtimectx->stackarena);

        if (channel->target_path == cgltf_animation_path_type_translation) {
            vec3s v0, v1;
            cgltf_accessor_read_float(sampler->output, k,     v0.raw, 3);
            cgltf_accessor_read_float(sampler->output, k + 1, v1.raw, 3);
            out_pose[node_idx].translation = glms_vec3_lerp(v0, v1, factor);
        } 
        else if (channel->target_path == cgltf_animation_path_type_rotation) {
            versors q0, q1;
            cgltf_accessor_read_float(sampler->output, k,     q0.raw, 4);
            cgltf_accessor_read_float(sampler->output, k + 1, q1.raw, 4);
            out_pose[node_idx].rotation = glms_quat_slerp(q0, q1, factor);
        } 
        else if (channel->target_path == cgltf_animation_path_type_scale) {
            vec3s s0, s1;
            cgltf_accessor_read_float(sampler->output, k,     s0.raw, 3);
            cgltf_accessor_read_float(sampler->output, k + 1, s1.raw, 3);
            out_pose[node_idx].scale = glms_vec3_lerp(s0, s1, factor);
        }
    }
}

INTERNAL void glmodel__internal__blend_poses(
    glmodel_TRS_state_t* pose_a, 
    const glmodel_TRS_state_t* pose_b, 
    float alpha, 
    size_t node_count
) {
    float cl_alpha = glm_clamp(alpha, 0.0f, 1.0f);

    for (size_t i = 0; i < node_count; ++i) {
        pose_a[i].translation = glms_vec3_lerp(pose_a[i].translation, pose_b[i].translation, cl_alpha);
        pose_a[i].rotation    = glms_quat_slerp(pose_a[i].rotation, pose_b[i].rotation, cl_alpha);
        pose_a[i].scale       = glms_vec3_lerp(pose_a[i].scale, pose_b[i].scale, cl_alpha);
    }
}

void glmodel_play_animation(glmodel_t *self, const u16 new_anim_idx)
{
    ASSERT(self && self->data);
    if (new_anim_idx >= self->data->animations_count) return;

    anim_state_machine_t *anim_sm = &self->anim_state_machine;

    if (anim_sm->current_anim_idx == new_anim_idx && !anim_sm->is_transitioning) return;

    anim_sm->target_anim_idx = new_anim_idx;
    anim_sm->target_time = 0.0f;
    anim_sm->transition_time = 0.0f;
    anim_sm->is_transitioning = true;
}

void glmodel_update(glmodel_t *self, f32 dt)
{
    ASSERT(self && self->data);

    anim_state_machine_t *anim_sm = &self->anim_state_machine;

    cgltf_data* data = self->data;
    if (data->animations_count == 0) return;

    anim_sm->current_time += dt;

    // Wrap time to loop seamlessly if duration is valid
    const f32 duration_a = glmodel_get_animation_duration(self, anim_sm->current_anim_idx);
    f32 sampled_time_a = anim_sm->current_time;
    if (duration_a > 0.0f) {
        sampled_time_a = fmodf(anim_sm->current_time, duration_a);
    }

    cgltf_skin* skin = (data->skins_count > 0) ? &data->skins[0] : NULL;

    // Reset matrix buffers to identity safely
    for (u32 idx = 0; idx < data->nodes_count; idx++) {
        self->world_matrices[idx] = glms_mat4_identity();
    }

    if (skin) {
        for (u32 idx = 0; idx < skin->joints_count; idx++) {
            self->uJoints_palette[idx] = glms_mat4_identity();
        }
    }

    // Allocate pose state arrays on stack arena
    glmodel_TRS_state_t *pose_a = (glmodel_TRS_state_t *)stackarena_push(global_runtimectx->stackarena, sizeof(glmodel_TRS_state_t) * data->nodes_count);
    glmodel_TRS_state_t *pose_b = (glmodel_TRS_state_t *)stackarena_push(global_runtimectx->stackarena, sizeof(glmodel_TRS_state_t) * data->nodes_count);

    const cgltf_animation* anim_a = &data->animations[anim_sm->current_anim_idx];
    glmodel__internal__sample_animation_pose(anim_a, sampled_time_a, data, pose_a);

    if (anim_sm->is_transitioning) {

        anim_sm->target_time += dt;
        anim_sm->transition_time += dt;

        const f32 duration_b = glmodel_get_animation_duration(self, anim_sm->target_anim_idx);
        f32 sampled_time_b = anim_sm->target_time;
        if (duration_b > 0.0f) {
            sampled_time_b = fmodf(anim_sm->target_time, duration_b);
        }

        const f32 alpha = anim_sm->transition_time / anim_sm->transition_duration;

        const cgltf_animation* anim_b = &data->animations[anim_sm->target_anim_idx];
        glmodel__internal__sample_animation_pose(anim_b, sampled_time_b, data, pose_b);

        // Blend pose_b into pose_a
        glmodel__internal__blend_poses(pose_a, pose_b, alpha, data->nodes_count);

        if (alpha >= 1.0f) {
            anim_sm->current_anim_idx       = anim_sm->target_anim_idx;
            anim_sm->current_time           = anim_sm->target_time;
            anim_sm->is_transitioning       = false;
        }
    }

    // Write final blended TRS from pose_a back into cgltf_nodes
    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* node = &data->nodes[i];
        node->has_matrix = 0; // Clear matrix flag so TRS components are used
        node->has_translation = 1;
        node->has_rotation = 1;
        node->has_scale = 1;

        glm_vec3_copy(pose_a[i].translation.raw, node->translation);
        glm_quat_copy(pose_a[i].rotation.raw, node->rotation);
        glm_vec3_copy(pose_a[i].scale.raw, node->scale);
    }

    // Free stack arena poses in LIFO order
    stackarena_pop(global_runtimectx->stackarena);
    stackarena_pop(global_runtimectx->stackarena);

    // Compute matrices
    glmodel__internal__compute_world_matrices_s(data, self->world_matrices);
    if (skin && self->uJoints_palette) {
        glmodel__internal__compute_joint_matrices_s(data, skin, self->world_matrices, self->uJoints_palette);
    }
}

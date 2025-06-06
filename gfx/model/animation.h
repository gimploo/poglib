#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/external/assimp/include/assimp/scene.h>

typedef struct {
    f32 time;          // Keyframe time in ticks
    vec3f_t value;     // Position value
} position_key_t;

typedef struct {
    f32 time;                   // Keyframe time in ticks
    quaternionf_t value;        // Rotation value (quaternion)
} rotation_key_t;

typedef struct {
    f32 time;          // Keyframe time in ticks
    vec3f_t value;     // Scaling value
} scaling_key_t;

typedef struct {
    const char *node_name; // Name of the node/bone this channel animates
    list_t position_keys;  // List of position_key_t
    list_t rotation_keys;  // List of rotation_key_t
    list_t scaling_keys;   // List of scaling_key_t
} node_anim_t;

typedef struct {
    const char *name;      // Animation name
    f32 duration;          // Duration in ticks
    f32 ticks_per_second;  // Ticks per second for time conversion
    list_t channels;       // List of node_anim_t (one per animated node/bone)
} animation_t;

typedef struct {
    list_t animations;     // List of animation_t
} animator_t;


animator_t          animator_init(void);
void                animator_load_all_animations(animator_t *self, const struct aiScene *scene);
animation_t *       animator_get_animation(const animator_t *self, const char *animation_label);
void                animator_destroy(animator_t *self);

animator_t animator_init(void)
{
    return (animator_t ){
        .animations = list_init(animation_t)
    };
}

void print_animation_labels(void *data)
{
    printf("\t%s\n", ((animation_t *)data)->name);
}

animation_t * animator_get_animation(const animator_t *self, const char *label)
{
    list_iterator(&self->animations, iter)
    {
        animation_t *a = iter;
        if (strcmp(a->name, label) == 0) {
            return iter;
        }
    }
    fprintf(stderr, "No animation for %s label was found, here are available animations: \n", label);
    list_print(&self->animations, print_animation_labels);
    ASSERT(false);
}

animation_t __animation_init(const char *name, const f32 duration, const f32 ticks_per_second)
{
    return (animation_t ) {
        .name = name,
        .duration = duration,
        .ticks_per_second = ticks_per_second,
        .channels = list_init(node_anim_t)
    };
}

void __animation_destroy(animation_t *animation)
{
    list_iterator(&animation->channels, iter) {
        node_anim_t *tmp = iter;
        list_destroy(&tmp->position_keys);
        list_destroy(&tmp->rotation_keys);
        list_destroy(&tmp->scaling_keys);
    }
    list_destroy(&animation->channels);
}
void animator_destroy(animator_t *self)
{
    list_iterator(&self->animations, iter) {
        animation_t *a = iter;
        __animation_destroy(a);
    }
    list_destroy(&self->animations);
}

void __load_channel_positions(const struct aiNodeAnim *node, list_t *pos)
{
    for (u32 i = 0; i < node->mNumPositionKeys; i++)
    {
        const struct aiVectorKey vk = node->mPositionKeys[i];
        const position_key_t n = {
            .time = vk.mTime,
            .value = (vec3f_t ){ .x = vk.mValue.x, .y = vk.mValue.y, .z = vk.mValue.z }
        };
        list_append(pos, n);
    }
}

void __load_channel_rotations(const struct aiNodeAnim *node, list_t *rot)
{
    for (u32 i = 0; i < node->mNumRotationKeys; i++)
    {
        const struct aiQuatKey vk = node->mRotationKeys[i];
        const rotation_key_t n = {
            .time = vk.mTime,
            .value = (quaternionf_t){ .x = vk.mValue.x, .y = vk.mValue.y, .z = vk.mValue.z, .w = vk.mValue.w }
        };
        list_append(rot, n);
    }
}

void __load_channel_scaling(const struct aiNodeAnim *node, list_t *scal)
{
    for (u32 i = 0; i < node->mNumScalingKeys; i++)
    {
        const struct aiVectorKey vk = node->mScalingKeys[i];
        const scaling_key_t n = {
            .time = vk.mTime,
            .value = (vec3f_t ){ vk.mValue.x, vk.mValue.y, vk.mValue.z }
        };
        list_append(scal, n);
    }
}

void __load_channels(const struct aiAnimation *animation, animation_t *self)
{
    for (u32 i = 0; i < animation->mNumChannels; i++)
    {
        const struct aiNodeAnim *node = animation->mChannels[i];
        node_anim_t n = {
            .node_name = node->mNodeName.data,
            .position_keys = list_init(position_key_t),
            .rotation_keys = list_init(rotation_key_t),
            .scaling_keys = list_init(scaling_key_t),
        };

        __load_channel_positions(node, &n.position_keys);
        __load_channel_rotations(node, &n.rotation_keys);
        __load_channel_scaling(node, &n.scaling_keys);

        list_append(&self->channels, n);
    }
}

void animator_load_all_animations(animator_t *self, const struct aiScene *scene)
{
    for (u32 i = 0; i < scene->mNumAnimations; i++)
    {
        const struct aiAnimation* ai = scene->mAnimations[i];

        animation_t item = __animation_init(
            ai->mName.data,
            ai->mDuration, 
            ai->mTicksPerSecond);

        __load_channels(ai, &item);

        list_append(&self->animations, item);
    }
}

// Helper function to find the position keyframe at or before the current time
static position_key_t __get_position_key(const list_t *position_keys, f32 time, f32 duration) {
    position_key_t result = {0};
    if (position_keys->len == 0) return result;

    // If time exceeds duration, loop or clamp based on animation behavior
    time = fmod(time, duration);

    position_key_t *prev = NULL;
    list_iterator(position_keys, iter) {
        position_key_t *key = iter;
        if (key->time > time) {
            if (prev) {
                // Interpolate between prev and key
                f32 t = (time - prev->time) / (key->time - prev->time);
                result.time = time;
                result.value = glms_vec3_lerp(prev->value, key->value, t);
                return result;
            } else {
                // Before first keyframe, return first
                return *(position_key_t *)list_get_value(position_keys, 0);
            }
        }
        prev = key;
    }
    // After last keyframe, return last
    return *(position_key_t *)list_get_value(position_keys, position_keys->len - 1);
}

// Helper function to find the rotation keyframe at or before the current time
static rotation_key_t __get_rotation_key(const list_t *rotation_keys, f32 time, f32 duration) {
    rotation_key_t result = { .value = QUATERNIONF_IDENTITY };
    if (rotation_keys->len == 0) return result;

    time = fmod(time, duration);

    rotation_key_t *prev = NULL;
    list_iterator(rotation_keys, iter) {
        rotation_key_t *key = iter;
        if (key->time > time) {
            if (prev) {
                // Interpolate between prev and key (slerp for quaternions)
                f32 t = (time - prev->time) / (key->time - prev->time);
                result.time = time;
                result.value = quaternionf_slerp(prev->value, key->value, t);
                return result;
            } else {
                return *(rotation_key_t *)list_get_value(rotation_keys, 0);
            }
        }
        prev = key;
    }
    return *(rotation_key_t *)list_get_value(rotation_keys, rotation_keys->len - 1);
}

// Helper function to find the scaling keyframe at or before the current time
static scaling_key_t __get_scaling_key(const list_t *scaling_keys, f32 time, f32 duration) {
    scaling_key_t result = { .value = (vec3f_t){1.0f, 1.0f, 1.0f} };
    if (scaling_keys->len == 0) return result;

    time = fmod(time, duration);

    scaling_key_t *prev = NULL;
    list_iterator(scaling_keys, iter) {
        scaling_key_t *key = iter;
        if (key->time > time) {
            if (prev) {
                // Interpolate between prev and key
                f32 t = (time - prev->time) / (key->time - prev->time);
                result.time = time;
                result.value = glms_vec3_lerp(prev->value, key->value, t);
                return result;
            } else {
                return *(scaling_key_t *)list_get_value(scaling_keys, 0);
            }
        }
        prev = key;
    }
    return *(scaling_key_t *)list_get_value(scaling_keys, scaling_keys->len - 1);
}

// Helper function to compute the transformation matrix for a node animation
matrix4f_t compute_node_transform(const node_anim_t *node_anim, f32 time, f32 duration) {
    position_key_t pos_key = __get_position_key(&node_anim->position_keys, time, duration);
    rotation_key_t rot_key = __get_rotation_key(&node_anim->rotation_keys, time, duration);
    scaling_key_t scale_key = __get_scaling_key(&node_anim->scaling_keys, time, duration);

    // Compute transformation: Translation * Rotation * Scale
    /*
    matrix4f_t translation = matrix4f_translation(pos_key.value);
    matrix4f_t rotation = quaternionf_to_matrix4f(rot_key.value);
    matrix4f_t scale = matrix4f_scale(scale_key.value);
    */
    matrix4f_t translation = glms_translate_make(pos_key.value);
    matrix4f_t rotation = glms_quat_mat4(rot_key.value);
    matrix4f_t scale = glms_scale_make(scale_key.value);

    return glms_mat4_mul(glms_mat4_mul(translation, rotation), scale);
}



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
void                animator_destroy(animator_t *self);

animator_t animator_init(void)
{
    return (animator_t ){
        .animations = list_init(animation_t)
    };
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
            .value = (vec3f_t ){ vk.mValue.x, vk.mValue.y, vk.mValue.z }
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


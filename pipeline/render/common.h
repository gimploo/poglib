#pragma once
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>

#define MAX_RENDER_BUCKETS_ALLOWED 255
#define INSTANCE_RENDER_BUCKET_CUBE_INDEX 0 
#define INSTANCE_RENDER_BUCKET_CAPSULE_INDEX 1 

typedef enum {
    RENDER_COMMAND_TYPE_CUSTOM = 0,
    RENDER_COMMAND_TYPE_CUBE = 1,
    RENDER_COMMAND_TYPE_CAPSULE = 2,
    RENDER_COMMAND_TYPE_CUSTOM_WITH_INSTANCING = 3,
    RENDER_COMMAND_TYPE_COUNT,
} rendercommand_types;

typedef struct {
    u8 bucket_ready_count;
    struct {
        bool is_ready;                  //NOTE: to know whether the list is initialized
        rendercommand_types type;
        list_t render_commands;
    } buckets[MAX_RENDER_BUCKETS_ALLOWED];
    arena_t arena;

    struct {
        glshader_t instance_shader;
    } internal;
} renderqueue_t;


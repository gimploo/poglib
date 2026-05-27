#pragma once
#include "poglib/util/assetmanager.h"
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>
#include <poglib/gfx/gl/instance-buffer.h>

#define MAX_RENDER_BUCKETS_ALLOWED 255
#define INSTANCE_RENDER_BUCKET_CUBE_INDEX 0 
#define INSTANCE_RENDER_BUCKET_CAPSULE_INDEX 1 

typedef enum {
    RENDER_COMMAND_DRAW_MODE_TRIANGLE = GL_TRIANGLES,
    RENDER_COMMAND_DRAW_MODE_LINES = GL_LINES,
    RENDER_COMMAND_DRAW_MODE_COUNT,
} rendercommand_draw_mode;


typedef struct {
    u8 bucket_ready_count;
    list_t buckets[MAX_RENDER_BUCKETS_ALLOWED];
    struct {
        glinstancebuffer_t instancebuffer;
        arena_t arena;
    } internal;
} renderqueue_t;


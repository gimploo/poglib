#pragma once
#include "poglib/util/assetmanager.h"
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>
#include <poglib/gfx/gl/instance-buffer.h>

#define MAX_RENDER_BUCKETS_ALLOWED 125

typedef struct renderqueue_t renderqueue_t;
struct renderqueue_t {

    list_t buckets[MAX_RENDER_BUCKETS_ALLOWED];
    struct {
        glinstancebuffer_t  instancebuffer;
        arena_t            *frame_arena;
    } internal;
};


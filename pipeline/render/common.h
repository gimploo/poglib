#pragma once
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>
#include <poglib/gfx/gl/instance-buffer.h>

typedef struct renderqueue_t renderqueue_t;
struct renderqueue_t {

    list_t buckets;
    struct {
        glinstancebuffer_t  instancebuffer;
        arena_t            *frame_arena;
    } internal;

};


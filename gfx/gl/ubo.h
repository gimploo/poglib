#pragma once
#include "./common.h"

typedef enum {
    GL_UBO_BINDING_IDX_LIGHT = 0,
} GL_UBO_BINDING_IDX_TYPE;

typedef struct {
    u32 id;
    u32 capacity;
} glubo_t;

glubo_t glubo_init(const u32 capacity, const u8 binding_idx)
{
    ASSERT(capacity > 0);

    u32 id;
    GL_CHECK(glGenBuffers(1, &id));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, id));
    GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, capacity, NULL, GL_DYNAMIC_DRAW));
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, binding_idx, id));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    return (glubo_t) {
        .id = id,
        .capacity = capacity
    };
}

void glubo_upload(glubo_t *const self, const buffer_t buffer)
{
    ASSERT(self);
    ASSERT(self->capacity >= buffer.size);
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, self->id));
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, buffer.size, buffer.raw_data));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void glubo_destroy(glubo_t *const self)
{
    ASSERT(self);
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    GL_CHECK(glDeleteBuffers(1, &self->id));
}


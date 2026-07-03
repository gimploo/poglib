#pragma once
#include "./common.h"

#define INSTANCE_BUFFER_COUNT 3

typedef struct {
    u32       ssbo_ids[INSTANCE_BUFFER_COUNT];
    u64       capacity;
    void *    mem_offsets[INSTANCE_BUFFER_COUNT];
    u8        active_index;
    struct {
        u64 current_offset;
    } internal;
} glinstancebuffer_t;


glinstancebuffer_t      glinstancebuffer_init(const u32 capacity);
void                    glinstancebuffer_bind(glinstancebuffer_t * const self, const u32 offset, const u32 size);
void                    glinstancebuffer_push(glinstancebuffer_t * const self, const void * const mem, const u32 size);
u32                     glinstancebuffer_get_current_offest(const glinstancebuffer_t * const self);
void                    glinstancebuffer_unbind(glinstancebuffer_t * const self);
void                    glinstancebuffer_destroy(glinstancebuffer_t * const self);

void glinstancebuffer_bind(glinstancebuffer_t * const self, const u32 offset, const u32 size)
{
    ASSERT(size > 0);
    ASSERT(self);

    GL_CHECK(glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER, 
        0, // instance buffer binding location
        self->ssbo_ids[self->active_index],
        offset, 
        size
    ));
}

glinstancebuffer_t glinstancebuffer_init(const u32 capacity)
{
    glinstancebuffer_t buf = {0};

    buf.capacity = capacity;
    buf.active_index = 0;

    GL_CHECK(glGenBuffers(INSTANCE_BUFFER_COUNT, buf.ssbo_ids));

    for (u8 i = 0; i < INSTANCE_BUFFER_COUNT; i++)
    {
        GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf.ssbo_ids[i]));
        GL_CHECK(glBufferStorage(GL_SHADER_STORAGE_BUFFER, capacity, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

        void *mem = NULL;
        GL_CHECK(mem = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, capacity, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
        ASSERT(mem);
        buf.mem_offsets[i] = mem;
    }

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    return buf;
}

void glinstancebuffer_unbind(glinstancebuffer_t * const self)
{
    self->internal.current_offset = 0;
    self->active_index = (self->active_index + 1) % INSTANCE_BUFFER_COUNT;
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

u32 glinstancebuffer_get_current_offest(const glinstancebuffer_t * const self)
{
    ASSERT(self);
    return self->internal.current_offset;
}

void glinstancebuffer_push(glinstancebuffer_t * const self, const void * const mem, const u32 size)
{
    if ((size + self->internal.current_offset) > self->capacity)
        eprint("Exceeded allowed GL instance buffer size, requires `%u` bytes but only `%u` bytes available", size, self->capacity - self->internal.current_offset);

    memcpy((u8 *)self->mem_offsets[self->active_index] + self->internal.current_offset, mem, size);
    self->internal.current_offset += size;
}

void glinstancebuffer_destroy(glinstancebuffer_t * const self)
{
    ASSERT(self);

    for (u8 i = 0; i < INSTANCE_BUFFER_COUNT; i++)
    {
        GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, self->ssbo_ids[i]));
        GL_CHECK(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
    }

    GL_CHECK(glDeleteBuffers(INSTANCE_BUFFER_COUNT, self->ssbo_ids));
    memset(self, 0, sizeof(*self));
}

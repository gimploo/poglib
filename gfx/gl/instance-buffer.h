#pragma once
#include "./common.h"

typedef struct {
    u32       ssbo_id;
    u64       capacity;
    void *    mem_offset;
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
        self->ssbo_id,
        offset, 
        size
    ));
}

glinstancebuffer_t glinstancebuffer_init(const u32 capacity)
{
    u32 global_instance_vbo;
    GL_CHECK(glGenBuffers(1, &global_instance_vbo));
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, global_instance_vbo));
    GL_CHECK(glBufferStorage(GL_SHADER_STORAGE_BUFFER, capacity, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

    void *mem_offset = NULL;
    GL_CHECK(mem_offset = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, capacity, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
    ASSERT(mem_offset);

    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    return (glinstancebuffer_t) {
        .mem_offset = mem_offset,
        .capacity = capacity,
        .ssbo_id = global_instance_vbo,
        .internal = {
            .current_offset = 0
        }
    };
}

void glinstancebuffer_unbind(glinstancebuffer_t * const self)
{
    self->internal.current_offset = 0;
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

    memcpy((u8 *)self->mem_offset + self->internal.current_offset, mem, size);
    self->internal.current_offset += size;
}

void glinstancebuffer_destroy(glinstancebuffer_t * const self)
{
    ASSERT(self);
    GL_CHECK(glDeleteBuffers(1, &self->ssbo_id));
    self->mem_offset = NULL;
}

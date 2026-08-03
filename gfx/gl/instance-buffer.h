#pragma once
#include "./common.h"
#include "./glconfig.h"

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
void                    glinstancebuffer_upload(glinstancebuffer_t * const self);
void                    glinstancebuffer_push(glinstancebuffer_t * const self, const void * const mem, const u32 size);
u32                     glinstancebuffer_get_current_offest(const glinstancebuffer_t * const self);
void                    glinstancebuffer_unbind(glinstancebuffer_t * const self);
void                    glinstancebuffer_destroy(glinstancebuffer_t * const self);

void glinstancebuffer_bind(glinstancebuffer_t * const self, const u32 offset, const u32 size)
{
    ASSERT(size > 0);
    ASSERT(self);

#ifdef __APPLE__
    // macOS (GL 4.1): no persistent SSBO mapping. Expose the per-instance
    // vertex attributes (locations 3..7) for the buffer staged in
    // glinstancebuffer_push. The actual upload is deferred to
    // glinstancebuffer_upload so it happens once per frame instead of once
    // per draw bucket (Apple's driver makes per-bucket glBufferSubData
    // calls disproportionately expensive).
    const u32 stride = sizeof(f32) * 20; // renderinstance_t == 5 * vec4

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, self->ssbo_ids[self->active_index]));

    for (u8 attr = 0; attr < 5; attr++)
    {
        const u32 loc = 3 + attr;
        GL_CHECK(glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, stride, (void *)(offset + attr * sizeof(f32) * 4)));
        GL_CHECK(glEnableVertexAttribArray(loc));
        GL_CHECK(glVertexAttribDivisor(loc, 1));
    }
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
#else
    GL_CHECK(glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER, 
        0, // instance buffer binding location
        self->ssbo_ids[self->active_index],
        offset, 
        size
    ));
#endif
}

void glinstancebuffer_upload(glinstancebuffer_t * const self)
{
    ASSERT(self);

#ifdef __APPLE__
    // One small upload per frame for everything staged by
    // glinstancebuffer_push this frame (see glinstancebuffer_bind).
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, self->ssbo_ids[self->active_index]));
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, self->internal.current_offset, self->mem_offsets[self->active_index]));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
#else
    (void)self;
#endif
}

glinstancebuffer_t glinstancebuffer_init(const u32 capacity)
{
    glinstancebuffer_t buf = {0};

    buf.capacity = capacity;
    buf.active_index = 0;

    GL_CHECK(glGenBuffers(INSTANCE_BUFFER_COUNT, buf.ssbo_ids));

    for (u8 i = 0; i < INSTANCE_BUFFER_COUNT; i++)
    {
#ifdef __APPLE__
        // macOS (GL 4.1): glBufferStorage + persistent mapping is 4.4-only.
        // Keep a CPU scratch copy and upload it in glinstancebuffer_bind.
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buf.ssbo_ids[i]));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, capacity, NULL, GL_DYNAMIC_DRAW));
        buf.mem_offsets[i] = malloc(capacity);
        ASSERT(buf.mem_offsets[i]);
#else
        GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf.ssbo_ids[i]));
        GL_CHECK(glBufferStorage(GL_SHADER_STORAGE_BUFFER, capacity, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

        void *mem = NULL;
        GL_CHECK(mem = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, capacity, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
        ASSERT(mem);
        buf.mem_offsets[i] = mem;
#endif
    }

#ifdef __APPLE__
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
#else
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
#endif

    return buf;
}

void glinstancebuffer_unbind(glinstancebuffer_t * const self)
{
    self->internal.current_offset = 0;
    self->active_index = (self->active_index + 1) % INSTANCE_BUFFER_COUNT;
#ifdef __APPLE__
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
#else
    GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
#endif
}

u32 glinstancebuffer_get_current_offest(const glinstancebuffer_t * const self)
{
    ASSERT(self);
    return self->internal.current_offset;
}

void glinstancebuffer_push(glinstancebuffer_t *const self, const void *const mem, const u32 size)
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
#ifdef __APPLE__
        free(self->mem_offsets[i]);
        self->mem_offsets[i] = NULL;
#else
        GL_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, self->ssbo_ids[i]));
        GL_CHECK(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
#endif
    }

    GL_CHECK(glDeleteBuffers(INSTANCE_BUFFER_COUNT, self->ssbo_ids));
    memset(self, 0, sizeof(*self));
}

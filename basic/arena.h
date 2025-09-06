#pragma once
#include "dbg.h"
#include "common.h"
#include "threads.h"

/*================================================================================
 *                      -- ARENA MEMORY ALLOCATOR --
================================================================================*/

struct free_chunks_t {
    u8 *memory;
    u32 size;
    struct free_chunks_t *next;
};
typedef struct free_chunks_t free_chunks_t;

struct arena_t {
    u8 *memory;
    u32 capacity;
    u32 size;
    struct {
        free_chunks_t *head;
        u32 count;
    } freelist;
    struct {
        mtx_t lock;
        struct arena_t *lifetime_owner;
    } meta;
};
typedef struct arena_t arena_t;

arena_t     arena_init(arena_t *, u32 capacity);
void *      arena_reserve(arena_t *self, u32 memory_size);
void        arena_clear(arena_t *self);
void        arena_giveback(arena_t *self, void *ptr, u32 size);
void        arena_destroy(arena_t *self);

#ifndef IGNORE_ARENA_IMPLEMENTATION

arena_t arena_init(arena_t *arena, u32 capacity)
{
    arena_t o = {
        .capacity = capacity,
        .size = 0,
        .memory = arena ? arena_reserve(arena, capacity) : calloc(capacity, sizeof(u8)),
        .freelist = {0},
        .meta = {
            .lifetime_owner = arena,
        }
    };
    mtx_init(&o.meta.lock, mtx_plain);
    return o;
}

void * __check_in_freelist(arena_t *self, u32 memory_size)
{
    if (self->freelist.count) {
        free_chunks_t *chunk = self->freelist.head;
        free_chunks_t *prev_chunk = NULL;
        for(u32 i = 0; i < self->freelist.count; i++) {
            ASSERT(self->freelist.head);
            ASSERT(chunk);
            if (memory_size == chunk->size) {
                if (prev_chunk) {
                    prev_chunk->next = chunk->next;
                } else {
                    self->freelist.head = chunk->next;
                }
                if(self->meta.lifetime_owner) {
                    arena_giveback(
                        self->meta.lifetime_owner, 
                        chunk, 
                        sizeof(free_chunks_t)
                    );
                } else {
                    free(chunk);
                }
                self->freelist.count--;
                return chunk->memory;
            }
            prev_chunk = chunk;
            chunk = chunk->next;
        }
    }
    return NULL;
}

void * arena_reserve(arena_t *self, u32 memory_size)
{
    void *mem = NULL;
    mtx_lock(&self->meta.lock);
    {
        if ((self->size + memory_size) > self->capacity)
        {
            eprint("Arena is full");
        }

        void *res_memory = __check_in_freelist(self, memory_size);
        if (res_memory) {
            mtx_unlock(&self->meta.lock);
            return res_memory;
        }

        mem = (void *)((u8 *)self->memory + self->size);
        self->size += memory_size;
    }
    mtx_unlock(&self->meta.lock);
    ASSERT(mem);
    return mem;
}

void arena_clear(arena_t *self)
{
    mtx_lock(&self->meta.lock);
        self->size = 0;
    mtx_unlock(&self->meta.lock);
}

void arena_giveback(arena_t *self, void *ptr, u32 size)
{
    ASSERT(self);
    ASSERT(ptr);
    ASSERT(size > 0);

    mtx_lock(&self->meta.lock);
    {
        free_chunks_t *chunk = self->freelist.head;
        while(chunk != NULL && chunk->next != NULL) {
            chunk = chunk->next;
        }

        free_chunks_t *new_chunk = self->meta.lifetime_owner 
            ? arena_reserve(self->meta.lifetime_owner, sizeof(free_chunks_t))
            : calloc(1, sizeof(free_chunks_t));

        ASSERT(new_chunk);
        *new_chunk = (free_chunks_t) {
            .memory = ptr,
            .size = size,
            .next = NULL
        };

        if (chunk) {
            chunk->next = new_chunk;
        } else {
            self->freelist.head = new_chunk;
        }
        self->freelist.count++;
    }
    mtx_unlock(&self->meta.lock);
}

void arena_destroy(arena_t *self)
{
    if (self->meta.lifetime_owner) {
        arena_giveback(self->meta.lifetime_owner, self->memory, self->capacity);
        mtx_destroy(&self->meta.lock);
        return;
    }

    mtx_lock(&self->meta.lock); 
    {
        free_chunks_t *cur = self->freelist.head;
        while(cur != NULL) {
            free_chunks_t *chunk = cur;
            cur = cur->next;
            free(chunk);
        }
        memset(self->memory, 0, self->capacity);
        free(self->memory);

    }
    mtx_unlock(&self->meta.lock);

    mtx_destroy(&self->meta.lock);
}
#endif

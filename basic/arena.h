#pragma once
#include "dbg.h"
#include "common.h"
#include "threads.h"
#include <stdint.h>

/*================================================================================
 *                      -- ARENA MEMORY ALLOCATOR --
================================================================================*/

//NOTE: supports only 16 byte alignment

typedef struct free_chunks_t free_chunks_t;
struct free_chunks_t {
    u8 *memory;
    free_chunks_t *next;
    u64 size;
};

struct arena_t {
    u8 *memory;
    u64 capacity;
    u64 size;
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

arena_t     arena_init(arena_t *, u64 capacity);
void *      arena_reserve(arena_t *self, u64 memory_size);
bool        arena_is_init(const arena_t * const self);
void *      arena_store(arena_t * const self, const void * const mem, const u64 mem_size);
void        arena_giveback(arena_t *self, const void *ptr, const u64 size);
void        arena_clear(arena_t *self);
void        arena_destroy(arena_t *self);

#ifndef IGNORE_ARENA_IMPLEMENTATION

void * arena__internal_reserve_memory_16byte_aligned(arena_t * const self, const u64 memory_size);

arena_t arena_init(arena_t *arena, u64 capacity)
{
    arena_t o = {
        .capacity = capacity,
        .size = 0,
        .memory = arena ? arena__internal_reserve_memory_16byte_aligned(arena, capacity) : calloc(capacity, sizeof(u8)),
        .freelist = {0},
        .meta = {
            .lifetime_owner = arena,
        }
    };
    mtx_init(&o.meta.lock, mtx_plain);
    return o;
}

void * arena__internal_check_in_freelist(arena_t *self, const u64 memory_size)
{
    void *memory = NULL;
    if (self->freelist.count) {
        free_chunks_t *chunk = self->freelist.head;
        free_chunks_t *prev_chunk = NULL;
        for(u32 i = 0; i < self->freelist.count; i++) {
            ASSERT(self->freelist.head);
            ASSERT(chunk);

            if (memory_size != chunk->size) {
                prev_chunk = chunk;
                chunk = chunk->next;
                continue;
            }

            memory = chunk->memory;

            if (prev_chunk) {
                prev_chunk->next = chunk->next;
            } else {
                self->freelist.head = chunk->next;
            }

            self->freelist.count--;
            free(chunk);
            return memory;
        }
    }
    return memory;
}

void * arena__internal_reserve_memory_16byte_aligned(arena_t * const self, const u64 memory_size)
{
    if ((self->capacity - self->size) < memory_size) {
        eprint("Ran out of memory, asking for `%i`bytes but `%i`bytes only available\n\tcapacity = %i | size = %i", memory_size, (self->capacity - self->size), self->capacity, self->size);
    }
    const u64 align = 16;
    void *mem = NULL;

    mtx_lock(&self->meta.lock);
    {
        const u64 current_ptr = (u64)self->memory + self->size;
        const u64 aligned_ptr = (current_ptr + (align - 1)) & ~(align - 1);
        const u64 padding = (u64)(aligned_ptr - current_ptr);
        const u64 offset = self->size + padding;

        if ((offset + memory_size) > self->capacity)
        {
            eprint("Arena is full");
            mtx_unlock(&self->meta.lock);
            return NULL;
        }

        void *res_memory = arena__internal_check_in_freelist(self, memory_size);
        if (res_memory) {
            memset(res_memory, 0, memory_size);
            mtx_unlock(&self->meta.lock);
            return res_memory;
        }

        mem = (void *)aligned_ptr;
        memset(mem, 0, memory_size);

        self->size = offset + memory_size;
    }
    mtx_unlock(&self->meta.lock);

    ASSERT(mem);
    if(((u64)(mem) & (16 - 1)) != 0) {
        eprint("Memory is not 16 byte aligned");
    };
    return mem;
}

void arena_clear(arena_t *self)
{
    mtx_lock(&self->meta.lock);
        self->size = 0;
    mtx_unlock(&self->meta.lock);
}


void arena_giveback(arena_t *self, const void *ptr, const u64 size)
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
            .memory = (u8 *)ptr,
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

void * arena_reserve(arena_t *self, u64 memory_size)
{
    ASSERT(memory_size);
    return arena__internal_reserve_memory_16byte_aligned(self, memory_size);
}

void * arena_store(arena_t * const self, const void * const mem, const u64 mem_size)
{
    ASSERT(mem_size > 0);
    ASSERT(mem);
    void *raw_mem = arena_reserve(self, mem_size);
    memcpy(raw_mem, mem, mem_size);
    return raw_mem;
}


bool arena_is_init(const arena_t * const self)
{
    return self->memory == NULL || self->capacity > 0;
}

#endif

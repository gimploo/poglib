#pragma once
#include "dbg.h"
#include "common.h"

/*================================================================================
 *                      -- ARENA MEMORY ALLOCATOR --
 ================================================================================*/

//NOTE: supports only 16 byte alignment

typedef struct arena_t arena_t;
typedef struct free_chunks_t free_chunks_t;

struct arena_t {
    u8 *memory;
    u64 capacity;
    u64 size;
    struct {
        free_chunks_t *head;
        u32 count;
    } freelist;
    struct {
        atomic_flag lock;
        struct arena_t *lifetime_owner;
    } meta;
};

struct free_chunks_t {
    void *memory;
    free_chunks_t *next;
    u64 size;
};

arena_t *   arena_init(arena_t *const, const u64 capacity);

#define arena_reserve(self, memory_size) \
    arena__internal__reserve_tracked((self), (memory_size), __FILE__, __LINE__, __func__)

void *      arena_store(arena_t *const self, const void *const mem, const u64 mem_size);
bool        arena_is_init(const arena_t *const self);
void        arena_giveback(arena_t *const self, void *const ptr, const u64 size);
void        arena_clear(arena_t *const self);
void        arena_destroy(arena_t *const self);

#ifndef IGNORE_ARENA_IMPLEMENTATION

void *      arena__internal__reserve_memory_16byte_aligned(arena_t * const self, const u64 memory_size);
void *      arena__internal__reserve_tracked(arena_t *const self, const u64 memory_size, const char *file, int line, const char *func);

void * arena__internal_check_in_freelist(arena_t *const self, const u64 memory_size)
{
    if (!self->freelist.count) {
        return NULL;
    }

    free_chunks_t *chunk        = self->freelist.head;
    free_chunks_t *prev_chunk   = NULL;
    void *memory                = NULL;

    for(u32 i = 0; i < self->freelist.count; i++) 
    {
        ASSERT(self->freelist.head);
        ASSERT(chunk);

        if (memory_size > chunk->size) {
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
        if (self->meta.lifetime_owner) {
            arena_giveback(self->meta.lifetime_owner, chunk, sizeof(free_chunks_t));
        } else {
            free(chunk);
        }
        return memory;
    }
    return memory;
}

void * arena__internal_reserve_memory_16byte_aligned(arena_t *const self, const u64 memory_size)
{
    if (!self->freelist.count && (self->capacity - self->size) < memory_size) {
        eprint("Ran out of memory, asking for `%li`bytes but `%li`bytes only available\n\tcapacity = %li | size = %li", memory_size, (self->capacity - self->size), self->capacity, self->size);
    }
    const u64 align = 16;
    void *mem = NULL;

    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
    {
        const u64 current_ptr = (u64)self->memory + self->size;
        const u64 aligned_ptr = (current_ptr + (align - 1)) & ~(align - 1);
        const u64 padding = (u64)(aligned_ptr - current_ptr);
        const u64 offset = self->size + padding;

        if ((offset + memory_size) > self->capacity)
        {
            eprint("Arena is full");
            atomic_flag_clear(&self->meta.lock);
            return NULL;
        }

        void *res_memory = arena__internal_check_in_freelist(self, memory_size);
        if (res_memory) {
            memset(res_memory, 0, memory_size);
            atomic_flag_clear(&self->meta.lock);
            return res_memory;
        }

        mem = (void *)aligned_ptr;
        memset(mem, 0, memory_size);

        self->size = offset + memory_size;
    }
    atomic_flag_clear(&self->meta.lock);

    ASSERT(mem);
    if(((u64)(mem) & (16 - 1)) != 0) {
        eprint("Memory is not 16 byte aligned");
    };
    return mem;
}

void * arena__internal__reserve_tracked(arena_t *const self, const u64 memory_size, const char *file, int line, const char *func)
{
    if (!self->meta.lifetime_owner)
        arena_logger_init(self, self->capacity, file, line);
    void *mem = arena__internal_reserve_memory_16byte_aligned(self, memory_size);
    if (mem) {
        arena_logger_log_alloc(self, memory_size, file, line, func);
    }
    return mem;
}

void * arena_store(arena_t * const self, const void * const mem, const u64 mem_size)
{
    ASSERT(mem_size > 0);
    ASSERT(mem);
    void *raw_mem = arena_reserve(self, mem_size);
    memcpy(raw_mem, mem, mem_size);
    return raw_mem;
}

arena_t * arena_init(arena_t *const arena, const u64 capacity)
{
    const u64 final_capacity = capacity + sizeof(arena_t);
    arena_t output = {
        .capacity = final_capacity,
        .size = 0,
        .memory = arena ? arena__internal_reserve_memory_16byte_aligned(arena, final_capacity) : calloc(final_capacity, sizeof(u8)),
        .freelist = {0},
        .meta = {
            .lifetime_owner = arena,
        },
    };
    atomic_flag_clear(&output.meta.lock);

    return arena_store(&output, &output, sizeof(arena_t));
}

void arena_giveback(arena_t *const self, void *const ptr, const u64 size)
{
    ASSERT(self);
    ASSERT(ptr);
    ASSERT(size > 0);
    ASSERT(arena_is_init(self));

    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
    {
        free_chunks_t *chunk = self->freelist.head;
        while(chunk != NULL && chunk->next != NULL) {
            chunk = chunk->next;
        }

        //NOTE: using raw allocator directly instead of arena_store to avoid
        //tracking internal freelist metadata as user allocations in the arena dump
        free_chunks_t *new_chunk;
        if (self->meta.lifetime_owner) {
            new_chunk = (free_chunks_t *)arena__internal_reserve_memory_16byte_aligned(
                self->meta.lifetime_owner, sizeof(free_chunks_t)
            );
            if (new_chunk) {
                new_chunk->memory = ptr;
                new_chunk->size = size;
                new_chunk->next = NULL;
            }
        } else {
            new_chunk = calloc(1, sizeof(free_chunks_t));
            if (new_chunk) {
                new_chunk->memory = ptr;
                new_chunk->size = size;
                new_chunk->next = NULL;
            }
        }
        ASSERT(new_chunk);

        if (chunk) {
            chunk->next = new_chunk;
        } else {
            self->freelist.head = new_chunk;
        }
        self->freelist.count++;
    }
    atomic_flag_clear(&self->meta.lock);
}

void arena_clear(arena_t *self)
{
    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
        self->size = sizeof(arena_t);
    atomic_flag_clear(&self->meta.lock);
    arena_logger_clear_log(self);
}

void arena_destroy(arena_t *const self)
{
    arena_logger_destroy(self);

    if (self->meta.lifetime_owner) {
        arena_giveback(self->meta.lifetime_owner, self->memory, self->capacity);
        return;
    }

    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
    {
        free_chunks_t *cur = self->freelist.head;
        while(cur != NULL) {
            free_chunks_t *chunk = cur;
            cur = cur->next;
            free(chunk);
        }
    }
    atomic_flag_clear(&self->meta.lock);
    free(self->memory);
}


bool arena_is_init(const arena_t *const self)
{
    return self && (self->memory != NULL || self->capacity > 0);
}

#endif

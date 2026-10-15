#pragma once
#include "dbg.h"
#include "common.h"

/*================================================================================
 *                      -- ARENA MEMORY ALLOCATOR --
 ================================================================================*/

//TODO: use virtual memory primitves to commit inital arena sizes over stdlib provided syscalls,
//this requires you to write custom impl for all platforms - couldnt find a lib that does it :(

//TODO: since we removed the atomic locks in this arena, we would need an assert to check for 
//accidental arena growth on other threads, and crash if found to avoid mem corruption

//#define ARENA_STARTING_BLOCK_SIZE (64 * 1024)
#define ARENA_STARTING_BLOCK_SIZE (1024)

typedef struct arena_mem_block_t arena_mem_block_t;
struct arena_mem_block_t {
    u8 *memory;
    u32 capacity;
    u32 size_used;
    arena_mem_block_t *next_block;
};

typedef struct arena_t arena_t;
struct arena_t {
    arena_mem_block_t *current_block;
    arena_mem_block_t first_block;
    struct {
        arena_t *lifetime_owner;
    } internal;
};

arena_t *       arena_init(arena_t *parent);
bool            arena_is_init(const arena_t *self);
void *          arena_reserve(arena_t *self, u32 size);
void *          arena_store(arena_t *self, const void *mem, u64 size);
void            arena_clear(arena_t *self);
void            arena_destroy(arena_t *self);

#ifndef IGNORE_ARENA_IMPLEMENTATION

INTERNAL u64 align_forward_16(u64 ptr) {
    return (ptr + 15) & ~((u64)15);
}

//TODO: maybe it wasnt a good idea to remove the capcity param in init - 
//for common use cases 64 KB is too small

arena_t *arena_init(arena_t *parent) 
{
    u8 *raw_mem = NULL;
    if (parent) {
        raw_mem = (u8*)arena_reserve(parent, ARENA_STARTING_BLOCK_SIZE);
    } else {
        raw_mem = (u8*)calloc(1, ARENA_STARTING_BLOCK_SIZE);
    }
    ASSERT(raw_mem);

    arena_t *arena = (arena_t *)raw_mem;
    arena->first_block.memory    = raw_mem;
    arena->first_block.capacity  = ARENA_STARTING_BLOCK_SIZE;
    arena->first_block.size_used = align_forward_16(sizeof(arena_t));
    arena->first_block.next_block = NULL;

    arena->current_block = &arena->first_block;
    arena->internal.lifetime_owner = parent;

    return arena;
}

INTERNAL arena_mem_block_t * arena__internal__get_available_block(arena_t *const arena, const u32 memory_size)
{
    arena_mem_block_t *curr = &arena->first_block;
    while(curr->next_block) {
        const u32 mem_left = curr->capacity - curr->size_used;
        if (mem_left >= memory_size) 
            return curr;
        curr = curr->next_block;
    }
    return curr;
}

void *arena_reserve(arena_t *self, u32 memory_size)
{
    if (!self || memory_size == 0) return NULL;

    arena_mem_block_t *memblock = arena__internal__get_available_block(self, memory_size);

    u64 current_ptr = (u64)memblock->memory + memblock->size_used;
    u64 aligned_ptr = align_forward_16(current_ptr);
    u32 padding     = (u32)(aligned_ptr - current_ptr);

    if (memblock->size_used + padding + memory_size > memblock->capacity) 
    {
        if (self->internal.lifetime_owner) {
            return arena_reserve(self->internal.lifetime_owner, memory_size);
        }

        const u32 required_size = memory_size + sizeof(arena_mem_block_t) + 16;
        const u32 block_size = required_size < ARENA_STARTING_BLOCK_SIZE 
            ? ARENA_STARTING_BLOCK_SIZE 
            : (required_size + ARENA_STARTING_BLOCK_SIZE - 1) & ~(ARENA_STARTING_BLOCK_SIZE - 1);

        u8 *new_mem = (u8 *)calloc(1, block_size);
        ASSERT(new_mem);

        arena_mem_block_t *new_block = (arena_mem_block_t *)new_mem;
        new_block->memory    = new_mem;
        new_block->capacity  = block_size;
        new_block->size_used = align_forward_16(sizeof(arena_mem_block_t));
        new_block->next_block = NULL;

        memblock->next_block = new_block;
        self->current_block = new_block;
        memblock = new_block;

        current_ptr = (uintptr_t)memblock->memory + memblock->size_used;
        aligned_ptr = align_forward_16(current_ptr);
        padding     = (u32)(aligned_ptr - current_ptr);
    }

    void *result = (void *)aligned_ptr;
    memblock->size_used += padding + memory_size;

    return result;
}

void *arena_store(arena_t *self, const void *mem, u64 mem_size) 
{
    ASSERT(mem_size > 0);
    void *dst = arena_reserve(self, (u32)mem_size);
    if (mem) memcpy(dst, mem, mem_size);
    return dst;
}

void arena_clear(arena_t *self) 
{
    if (!self) return;

    arena_mem_block_t *curr = &self->first_block;
    while (curr) {
        u32 header_offset = (curr == &self->first_block) ? sizeof(arena_t) : sizeof(arena_mem_block_t);
        u32 reset_offset  = (u32)align_forward_16(header_offset);

        memset(curr->memory + reset_offset, 0, curr->size_used - reset_offset);
        curr->size_used = reset_offset;
        curr = curr->next_block;
    }
    self->current_block = &self->first_block;
}

void arena_destroy(arena_t *self)
{
    if (!self) return;

    arena_mem_block_t *curr = self->first_block.next_block;
    while (curr) {
        arena_mem_block_t *next = curr->next_block;
        if (!self->internal.lifetime_owner) {
            free(curr->memory);
        }
        curr = next;
    }

    if (!self->internal.lifetime_owner) {
        free(self->first_block.memory);
    }
}

bool arena_is_init(const arena_t *self) {
    return self && self->first_block.memory != NULL;
}

#endif

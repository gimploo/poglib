#pragma once
#include "./common.h"
#include "./ds/stack.h"

typedef struct stackarena_t stackarena_t;
struct stackarena_t {
    stack_t requestedsizes;
    u8      *mem;
    u64     capacity;
    u64     top;
};

stackarena_t * stackarena_init(const u64 capacity)
{
    const u64 mem_size = capacity + sizeof(stackarena_t);
    void *const buffer = calloc(1, mem_size);
    ASSERT(buffer);
    const stackarena_t arena = {
        .capacity           = capacity,
        .mem                = (u8 *)((u64)buffer + sizeof(stackarena_t)),
        .requestedsizes     = stack_init(capacity, u32, NULL),
        .top                = (u64)buffer + sizeof(stackarena_t),
    };

    memcpy(buffer, &arena, sizeof(stackarena_t));
    return buffer;
}

u8 * stackarena_push(stackarena_t *const self, const u64 size)
{
    ASSERT(self);
    ASSERT(size > 0);

    const u64 space_left = self->capacity - ((u64)self->top - (u64)self->mem);
    if (size > space_left) {
        eprint("stack arena is full, requested %llu from already filled size %llu", size, self->capacity);
    }

    u8 *const mem = (void *)self->top;
    self->top += size;

    stack_push(&self->requestedsizes, &size, sizeof(u32));
    return mem;
}

void stackarena_pop(stackarena_t *const self)
{
    ASSERT(self);
    if (self->top == (u64)self->mem)            eprint("trying to pop from an empty arena");

    const u32 last_requested_size = *(u32 *)stack_peek(&self->requestedsizes);
    stack_pop(&self->requestedsizes);
    self->top = self->top - last_requested_size;
}

void stackarena_destroy(stackarena_t *const self)
{
    ASSERT(self);
    stack_destroy(&self->requestedsizes);
    free(self);
}



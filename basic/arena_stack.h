#pragma once
#include "./common.h"
#include "./ds/stack.h"

typedef struct stackarena_t stackarena_t;
struct stackarena_t {
    u8      *mem;
    u64     capacity;
    u64     top;
};

stackarena_t * stackarena_init(const u64 capacity)
{
    void *const buffer = calloc(1, capacity + sizeof(stackarena_t));
    ASSERT(buffer);
    const stackarena_t arena = {
        .capacity = capacity,
        .mem = (u8 *)((u64)buffer + sizeof(stackarena_t)),
        .top = (u64)buffer + sizeof(stackarena_t),
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
        eprint("stack arena is full");
    }

    u8 *const mem = (void *)self->top;
    self->top += size;
    return mem;
}

void stackarena_pop(stackarena_t *const self, const u64 size)
{
    ASSERT(self);
    if (self->top == (u64)self->mem)            eprint("trying to pop from an empty arena");
    if ((self->top - size) < (u64)self->mem)    eprint("somewhere above you missed to pop");

    self->top = self->top - size;
}

void stackarena_destroy(stackarena_t *const self)
{
    ASSERT(self);
    free(self);
}



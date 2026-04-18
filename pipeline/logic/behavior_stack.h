#pragma once
#include "poglib/basic/ds/stack.h"
#include <poglib/basic.h>

typedef struct behaviorstack_t behaviorstack_t;

typedef struct {
    u16  state_type;
    void (*start)(behaviorstack_t * const);
    void (*update)(behaviorstack_t * const);
    void (*exit)(behaviorstack_t * const);
} behaviorstack_state_t;

struct behaviorstack_t {
    stack_t stack;
};

behaviorstack_t             behaviorstack_init(arena_t * const arena);
void                        behaviorstack_pop_state(behaviorstack_t * const self);
void                        behaviorstack_push_state(behaviorstack_t * const self, const behaviorstack_state_t state);
void                        behaviorstack_update(behaviorstack_t * const self);

#ifndef IGNORE_BEHAVIOR_STATCK_IMPLEMENTATION

behaviorstack_t behaviorstack_init(arena_t * const arena)
{
    ASSERT(arena);
    return (behaviorstack_t){
        .stack = stack_init(10, behaviorstack_state_t, arena),
    };
}

void behaviorstack_push_state(behaviorstack_t * const self, const behaviorstack_state_t state)
{
    stack_push(&self->stack, state);
    state.start(self);
}

void behaviorstack_pop_state(behaviorstack_t * const self)
{
    behaviorstack_state_t *state = stack_peak(&self->stack);
    stack_pop(&self->stack);
    state->exit(self);
}

void behaviorstack_update(behaviorstack_t * const self)
{
    if (!self->stack.len) return;

    behaviorstack_state_t *state = stack_peak(&self->stack);
    state->update(self);
}
#endif

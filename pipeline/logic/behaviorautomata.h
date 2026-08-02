#pragma once
#include "poglib/basic/ds/stack.h"
#include "poglib/input/commandqueue.h"
#include <poglib/basic.h>
#include <poglib/ecs.h>

//NOTE: each state is responsible to pop / push state on the stack
//this automata pure as that it only updates the topmost stack, reason
//i decided to avoid automata to manage is to avoid complexity and keep the
//logic as linear as possible

typedef struct behaviorautomata_t behaviorautomata_t;
typedef struct behaviorautomata_state_t behaviorautomata_state_t;
typedef struct behaviorautomata_ctx_t behaviorautomata_ctx_t;

struct behaviorautomata_ctx_t {
    void    *payload;
};

struct behaviorautomata_state_t {
    u16                             state_type;
    behaviorautomata_ctx_t          ctx;
    void (*start)(behaviorautomata_t *const, behaviorautomata_ctx_t *const ctx);
    void (*update)(behaviorautomata_t *const, const commandqueue_t * const queue, behaviorautomata_ctx_t * const ctx, const f32 delta_time);
    void (*exit)(behaviorautomata_t *const, behaviorautomata_ctx_t *const ctx);
};

struct behaviorautomata_t {
    ds_stack_t stack;
};

behaviorautomata_t          behaviorautomata_init(arena_t * const arena);
void                        behaviorautomata_pop_state(behaviorautomata_t * const self);
void                        behaviorautomata_push_state(behaviorautomata_t * const self, behaviorautomata_state_t state);
void                        behaviorautomata_update(behaviorautomata_t * const self, const commandqueue_t * const queue, const f32 delta_time);

#ifndef IGNORE_BEHAVIORAUTOMATA_IMPLEMENTATION

behaviorautomata_t behaviorautomata_init(arena_t * const arena)
{
    ASSERT(arena);
    return (behaviorautomata_t){
        .stack = stack_init(10, behaviorautomata_state_t, arena),
    };
}

behaviorautomata_state_t behaviorautomata_peek_state(const behaviorautomata_t *const self)
{
    if (stack_is_empty(&self->stack)) {
        return (behaviorautomata_state_t){0};
    }
    return *(behaviorautomata_state_t *)stack_peek(&self->stack);
}

void behaviorautomata_push_state(behaviorautomata_t *const self, behaviorautomata_state_t state)
{
    if (!stack_is_empty(&self->stack)) {
        behaviorautomata_state_t *const current_state = stack_peek(&self->stack);
        current_state->exit(self, &current_state->ctx);
    }

    stack_push(&self->stack, &state, sizeof(state));
    state.start(self, &state.ctx);
}

void behaviorautomata_pop_state(behaviorautomata_t * const self) {
    if (stack_is_empty(&self->stack)) return;

    behaviorautomata_state_t *state = stack_peek(&self->stack);
    stack_pop(&self->stack);
    state->exit(self, &state->ctx);

    state = stack_peek(&self->stack);
    state->start(self, &state->ctx);
}

void behaviorautomata_update(behaviorautomata_t * const self, const commandqueue_t * const queue, const f32 delta_time) {
    if (!self->stack.len) return;

    behaviorautomata_state_t *state = stack_peek(&self->stack);
    state->update(self, queue, &state->ctx, delta_time);

    //logging("running state = %i", state->state_type);
}
#endif

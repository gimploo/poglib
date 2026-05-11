#pragma once
#include "poglib/basic/ds/stack.h"
#include "poglib/poggen/input/commandqueue.h"
#include "poglib/poggen/input/commandregistry.h"
#include <poglib/basic.h>
#include <poglib/ecs.h>


//NOTE: each state is responsible to pop / push state on the stack
//this automata pure as that it only updates the topmost stack, reason
//i decided to avoid automata to manage is to avoid complexity and keep the
//logic as linear as possible

typedef struct behaviorautomata_t behaviorautomata_t;

typedef struct {
    ecs_t   *ecs;
    void    *payload;
} behaviorautomata_ctx_t;

typedef struct {
    u16                             state_type;
    behaviorautomata_ctx_t          ctx;
    void (*start)(behaviorautomata_t * const, behaviorautomata_ctx_t *const ctx);
    void (*update)(behaviorautomata_t * const, const commandqueue_t * const queue, behaviorautomata_ctx_t * const ctx, const f32 delta_time);
    void (*exit)(behaviorautomata_t * const, behaviorautomata_ctx_t *const ctx);
} behaviorautomata_state_t;

struct behaviorautomata_t {
    stack_t stack;
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

void behaviorautomata_push_state(behaviorautomata_t * const self, behaviorautomata_state_t state)
{
    ASSERT(state.ctx.ecs);

    stack_push(&self->stack, state);
    state.start(self, &state.ctx);
}

void behaviorautomata_pop_state(behaviorautomata_t * const self) {
    behaviorautomata_state_t *state = stack_peek(&self->stack);
    stack_pop(&self->stack);
    state->exit(self, &state->ctx);
}

void behaviorautomata_update(behaviorautomata_t * const self, const commandqueue_t * const queue, const f32 delta_time) {
    if (!self->stack.len) return;

    behaviorautomata_state_t *state = stack_peek(&self->stack);
    state->update(self, queue, &state->ctx, delta_time);
}
#endif

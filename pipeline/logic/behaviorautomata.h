#pragma once
#include "poglib/basic/ds/stack.h"
#include "poglib/poggen/input/commandqueue.h"
#include "poglib/poggen/input/commandregistry.h"
#include <poglib/basic.h>


//NOTE: each state is responsible to pop / push state on the stack
//this automata pure as that it only updates the topmost stack, reason
//i decided to avoid automata to manage is to avoid complexity and keep the
//logic as linear as possible

typedef struct behaviorautomata_t behaviorautomata_t;

typedef struct {
    //TODO: add ecs after its ready!
    union {
        void *ctx;
    };
} behaviorautomata_stateconfig_t;

typedef struct {
    u16                             state_type;
    behaviorautomata_stateconfig_t  config;
    void (*start)(behaviorautomata_t * const, void *const ctx);
    void (*update)(behaviorautomata_t * const, const commandqueue_t * const queue, void * const ctx);
    void (*exit)(behaviorautomata_t * const, void *const ctx);
} behaviorautomata_state_t;

struct behaviorautomata_t {
    stack_t stack;
};

behaviorautomata_t          behaviorautomata_init(arena_t * const arena);
void                        behaviorautomata_pop_state(behaviorautomata_t * const self);
void                        behaviorautomata_push_state(behaviorautomata_t * const self, behaviorautomata_state_t state);
void                        behaviorautomata_update(behaviorautomata_t * const self, const commandqueue_t * const queue);

#ifndef IGNORE_BEHAVIORAUTOMATA_IMPLEMENTATION

behaviorautomata_t behaviorautomata_init(arena_t * const arena) {
    ASSERT(arena);
    return (behaviorautomata_t){
        .stack = stack_init(10, behaviorautomata_state_t, arena),
    };
}

void behaviorautomata_push_state(behaviorautomata_t * const self, const behaviorautomata_state_t state) {
    stack_push(&self->stack, state);
    state.start(self, state.config.ctx);
}

void behaviorautomata_pop_state(behaviorautomata_t * const self) {
    behaviorautomata_state_t *state = stack_peek(&self->stack);
    stack_pop(&self->stack);
    state->exit(self, state->config.ctx);
}

void behaviorautomata_update(behaviorautomata_t * const self, const commandqueue_t * const queue) {
    if (!self->stack.len) return;

    behaviorautomata_state_t *state = stack_peek(&self->stack);
    state->update(self, queue, state->config.ctx);
}
#endif

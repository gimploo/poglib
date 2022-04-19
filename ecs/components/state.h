#pragma once
#include "../../basic.h"

typedef struct c_state_t {

    char *state;
    void (*update)(struct c_state_t *, const char *label);

} c_state_t ;

c_state_t c_state(const char *state);

#ifndef IGNORE_C_STATE_IMPLEMENTATION

void __c_state_update(c_state_t *self, const char *newstate)
{
    self->state = (char *)newstate;
}

c_state_t c_state(const char *state)
{
    return (c_state_t ) {
        .state = (char *)state,
        .update = __c_state_update
    };
}

#endif

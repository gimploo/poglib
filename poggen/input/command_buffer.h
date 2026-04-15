#pragma once
#include <poglib/basic.h>
#include "./commandregistry.h"

#define MAX_ALLOWED_COMMANDS_PER_FRAME 10

typedef u16 command_t;

typedef struct {
    commandregistry_t registry;
    queue_t commands;
} commandqueue_t;


commandqueue_t      commandqueue_init(arena_t * const arena, const commandregistry_t registry);
void                commandqueue_clear(commandqueue_t * const self);


#ifndef IGNORE_COMMAND_BUFFER_IMPLEMENTATION

commandqueue_t commandqueue_init(arena_t * const arena, const commandregistry_t registry)
{
    ASSERT(arena);
    return(commandqueue_t) {
        .commands = queue_init(MAX_ALLOWED_COMMANDS_PER_FRAME, command_t, arena),
        .registry = registry
    };
}

void commandqueue_listen_for_input(commandqueue_t * const self)
{
    
}

void commandqueue_clear(commandqueue_t * const self)
{
    queue_clear(&self->commands);
}


#endif

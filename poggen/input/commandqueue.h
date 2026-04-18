#pragma once
#include <poglib/basic.h>
#include "./commandregistry.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"
#include "poglib/application/window/sdl_window.h"

#define MAX_ALLOWED_COMMANDS_PER_FRAME 10

typedef u16 command_t;

typedef struct {
    commandregistry_t registry;
    queue_t commands;
} commandqueue_t;


commandqueue_t      commandqueue_init(arena_t * const arena, const commandregistry_t registry);
void                commandqueue_sync_input(commandqueue_t * const self);


#ifndef IGNORE_COMMAND_BUFFER_IMPLEMENTATION

commandqueue_t commandqueue_init(arena_t * const arena, const commandregistry_t registry) {
    ASSERT(arena);
    return(commandqueue_t) {
        .commands = queue_init(MAX_ALLOWED_COMMANDS_PER_FRAME, command_t, arena),
        .registry = registry
    };
}

void commandqueue_sync_input(commandqueue_t * const self) {
    const u8 *keyboard_buffer = SDL_GetKeyboardState(NULL);
    const sdl_mousebuttontype mouse_state = global_window->mouse.button;
    const commandregistry_t *commands = &self->registry;

    queue_clear(&self->commands);

    for (u16 command_type = 0; command_type < commands->count; command_type++) {
        if (self->registry.registry[command_type].type == COMMANDINPUTKEY_TYPE_KEYBOARD
            && keyboard_buffer[self->registry.registry[command_type].sdl_keyboard_key]) {
            queue_put(&self->commands, command_type);
            printf("KB Tracked %i\n", command_type);
        } else if (self->registry.registry[command_type].type == COMMANDINPUTKEY_TYPE_MOUSE 
            && mouse_state == self->registry.registry[command_type].sdl_mouse_key) {
            queue_put(&self->commands, command_type);
            printf("MS Tracked %i\n", command_type);
        }
    }
}



#endif

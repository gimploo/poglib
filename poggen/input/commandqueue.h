#pragma once
#include <poglib/basic.h>
#include "./commandregistry.h"
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

#define MAX_ALLOWED_COMMANDS_PER_FRAME 10

typedef u16 command_t;

typedef struct {

    commandregistry_t registry;

    //TODO: revaluate whether queue is needed and keep the bitmask as the single SOT
    queue_t commands; 

    struct {
        u16 bitmask;
    } internal;

} commandqueue_t;


commandqueue_t      commandqueue_init(arena_t * const arena, const commandregistry_t registry);
void                commandqueue_sync_input(commandqueue_t * const self);
u16                 commandqueue_get_commands_as_bitmask(const commandqueue_t * const self);


#ifndef IGNORE_COMMAND_BUFFER_IMPLEMENTATION

commandqueue_t commandqueue_init(arena_t * const arena, const commandregistry_t registry) {
    ASSERT(arena);
    return(commandqueue_t) {
        .commands = queue_init(MAX_ALLOWED_COMMANDS_PER_FRAME, command_t, arena),
        .registry = registry
    };
}

void cq__internal_print_u16_bitmask(uint16_t mask) {
    for (int i = 15; i >= 0; i--) {
        uint16_t bit = (mask >> i) & 1;
        printf("%u", bit);
        if (i % 4 == 0 && i != 0) printf(" "); // Add spacing for readability
    }
    printf("\n");
}

void commandqueue_sync_input(commandqueue_t * const self) {
    const u8 *keyboard_buffer = SDL_GetKeyboardState(NULL);
    const commandregistry_t *commands = &self->registry;

    self->internal.bitmask = 0;
    queue_clear(&self->commands);

    //printf("-------------------- NEW BATCH --------------------------------\n");
    for (u16 command_type = 0; command_type < commands->count; command_type++) {
        if (self->registry.registry[command_type].type == COMMANDINPUTKEY_TYPE_KEYBOARD
            && keyboard_buffer[self->registry.registry[command_type].sdl_keyboard_key]) {
            queue_put(&self->commands, command_type);
            self->internal.bitmask |= (1 << command_type);
            //printf("KB Tracked %i\n", command_type);
        } else if (
            self->registry.registry[command_type].type == COMMANDINPUTKEY_TYPE_MOUSE 
            && window_mouse_button_is_pressed(
                global_window, 
                self->registry.registry[command_type].sdl_mouse_key)
        ) {
            queue_put(&self->commands, command_type);
            self->internal.bitmask |= (1 << command_type);
            //printf("MS Tracked %i\n", command_type);
        }
        //printf("Bitmask: ");cq__internal_print_u16_bitmask(self->internal.bitmask);
    }
    //printf("-------------------- END BATCH --------------------------------\n");
}


u16 commandqueue_get_commands_as_bitmask(const commandqueue_t * const self) {
    ASSERT(self);
    return self->internal.bitmask;
}

#endif

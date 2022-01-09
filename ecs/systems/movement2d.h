#pragma once

#include "../entitymanager.h"
#include "../components.h"


void s_movement2d(entitymanager_t *manager, entity_t *player, f32 dt)
{
    assert(manager);
    assert(player);

    c_input_t *input = (c_input_t *)entity_get_component(player, c_input_t );
    if (input == NULL) eprint("entity requires input component for movement system");

    c_transform_t *transform = (c_transform_t *)entity_get_component(player, c_transform_t );
    if (transform == NULL) eprint("entity is missing transform component");

    c_shape2d_t *shape = (c_shape2d_t *)entity_get_component(player, c_shape2d_t );
    if (shape == NULL) eprint("entity is missing shape2d component");
    
    window_t *win = input->win;
    assert(win);

    if (window_keyboard_is_key_pressed(win, SDLK_d)) {

        transform->position = vec3f_add(
                transform->position, 
                (vec3f_t ){ transform->velocity.cmp[X] * dt, 0.0f, 0.0f });
    }

    if (window_keyboard_is_key_pressed(win, SDLK_a)) {

        transform->position = vec3f_add(
                transform->position, 
                (vec3f_t ){ -transform->velocity.cmp[X] *dt , 0.0f, 0.0f });
    }

    if (window_keyboard_is_key_pressed(win, SDLK_w)) {

        transform->position = vec3f_add(
                transform->position, 
                (vec3f_t ){ 0.0f, transform->velocity.cmp[Y] *dt, 0.0f });

    }

    if (window_keyboard_is_key_pressed(win, SDLK_s)) {

        transform->position = vec3f_add(
                transform->position, 
                (vec3f_t ){ 0.0f, -transform->velocity.cmp[Y] *dt, 0.0f });
    } 
    

    c_shape2d_update(shape);
}





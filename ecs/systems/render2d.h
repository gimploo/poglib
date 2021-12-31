#pragma once

#include "../../simple/glrenderer2d.h"
#include "../components/sprite.h"
#include "../components/shape.h"
#include "../components/shader.h"
#include "../components/texture.h"

#include "../entitymanager.h"


typedef struct s_renderer2d_t s_renderer2d_t ;




#ifndef IGNORE_RENDER2D_SYSTEM_IMPLEMENTATION


void s_renderer2d_draw(entitymanager_t *manager)
{
    assert(manager);

    glquad_t quad[MAX_QUAD_CAPACITY_PER_DRAW_CALL] = {0};
    u64 quad_count = -1;


    bool flag = false;
    glshader_t *shader = NULL;
    gltexture2d_t *texture = NULL;

    list_t *entities = &manager->entities;
    assert(entities->len != 0);

    for (u64 i = 0; i < entities->len; i++)
    {
        entity_t *e = (entity_t *)list_get_element_by_index(entities, i);
        assert(e);

        c_shape2d_t *shape = (c_shape2d_t *)entity_get_component(e, c_shape2d_t );
        c_sprite_t *sprite  = (c_sprite_t *)entity_get_component(e, c_sprite_t );
        if (sprite)

        if (shape->type == CST_SQUARE ) 
            memcpy(&quad[++quad_count], sprite->vertices, sizeof(sprite->vertices));

        else 
            eprint("todo batching different shapes");
    }
    if (!flag) return;

    glrenderer2d_t rd = glrenderer2d_init();

    glbatch_t batch = glbatch_init();
    glrenderer2d_draw_from_batch(&sys->renderer, &batch);

    glbatch_destroy(&batch);
    glrenderer2d_destroy(&rd);
}


#endif

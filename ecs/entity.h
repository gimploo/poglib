#pragma once
#include "../basic.h"
#include "../math/la.h"
#include "../ds/hashtable.h"
#include "../ds/list.h"
#include "../simple/gl/types.h"




typedef u64 entity_type;
typedef struct entity_t entity_t;


entity_t *      entity_init(entity_type tag, vec3f_t position);
void            entity_destroy(entity_t *e);








#ifndef IGNORE_ENTITY_IMPLEMENTATION


struct entity_t {

    // Members
    const u64   id;
    entity_type tag; 
    bool        is_alive;
    vec3f_t     position;

    // dynamic list that holds pointers to components
    list_t      components;

    // opengl data (cache)
    glvertex_t *__vertices;
};

void entity_destroy(entity_t *e)
{
    assert(e);

    list_destroy(&e->components);
    free(e->__vertices);

    free(e);
}

entity_t * entity_init(entity_type tag, vec3f_t position)
{
    entity_t *e = (entity_t *)malloc(1 * sizeof(entity_t));
    assert(e);

    entity_t tmp = (entity_t ) {
        .id = (u64)e,
        .tag = tag,
        .is_alive = true,
        .position = position,
        .components = list_init(4, void *),
        .__vertices = NULL,
    };

    memcpy(e, &tmp, sizeof(tmp));

    return e;
}


#endif

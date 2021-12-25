#pragma once
#include "../basic.h"
#include "../math/la.h"
#include "../ds/hashtable.h"
#include "../ds/list.h"


typedef u64 entity_type;
typedef struct entity_t entity_t;
typedef struct entity_manager_t entity_manager_t;

entity_manager_t    entity_manager_init(const u64 total_entity_types);
entity_t *          entity_manager_add_entity(entity_manager_t *manager, entity_type tag);
void                entity_manager_destroy(entity_manager_t *manager);






#ifndef IGNORE_ENTITY_IMPLEMENTATION

struct entity_t {

    // Members
    const u64   id;
    entity_type tag; 
    bool        alive;

    // Components
    // ...
};

entity_t * entity_init(entity_type tag)
{
    entity_t *e = (entity_t *)malloc(1 * sizeof(entity_t));
    assert(e);

    entity_t tmp = (entity_t ) {
        .id = (u64)e,
        .tag = tag,
        .alive = true,
    };

    memcpy(e, &tmp, sizeof(tmp));

    return e;
}

#define entity_destroy(PENTITY) free(PENTITY)


struct entity_manager_t {

    list_t      entities;
    list_t      *entitymap;
    u64         total_entities;

};


entity_manager_t entity_manager_init(const u64 total_entity_types)
{
    assert(total_entity_types > 0);

    return (entity_manager_t ) {
        .entities = list_init(10),
        .entitymap = (list_t *)calloc(total_entity_types, sizeof(list_t )),
        .total_entities = 0
    };
}

entity_t * entity_manager_add_entity(entity_manager_t *manager, entity_type tag)
{
    assert(manager);

    entity_t *e = entity_init(tag);
    assert(e);

    list_append(&manager->entities, e);
    list_append(&manager->entitymap[tag], e);

    manager->total_entities++;

    return e;
}

void entity_manager_destroy(entity_manager_t *manager)
{
    assert(manager);

    for (u64 i = 0; i < list_get_length(&manager->entities); i++)
    {
        free(list_get_element_by_index(&manager->entities, i));
    }

    list_destroy(&manager->entities);
    free(manager->entitymap);
    manager->total_entities = 0;
}
#endif

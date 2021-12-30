#pragma once
#include "entity.h"





typedef struct entitymanager_t entitymanager_t;

entitymanager_t     entitymanager_init(const u64 total_entity_types);

entity_t *          entitymanager_add_entity(entitymanager_t *manager, entity_t * e);
void                entitymanager_update(entitymanager_t *manager);

void                entitymanager_destroy(entitymanager_t *manager);

#define             entitymanager_get_all_entities_by_tag(PMANAGER, TAG)   (list_t *)list_get_element_by_index(&(PMANAGER)->entitymap, (TAG))
#define             entitymanager_get_all_entities(PMANAGER)               &(PMANAGER)->entities






#ifndef IGNORE_ENTITY_MANAGER_IMPLEMENTATION

struct entitymanager_t {

    //An list of all entities
    list_t      entities;

    //An list of list of entites differentiated by types
    list_t      entitymap; 

    u64         total_entities;

};


entitymanager_t entitymanager_init(const u64 total_entity_types)
{
    assert(total_entity_types > 0);

    list_t map = list_init(total_entity_types, sizeof(list_t ));
    for (int i = 0; i < total_entity_types; i++)
    {
        list_t tmp = list_init(4, entity_t *);
        list_append(&map, tmp);
    }

    return (entitymanager_t ) {
        .entities = list_init(8, entity_t *),
        .entitymap = map,
        .total_entities = 0,
    };
}

entity_t * entitymanager_add_entity(entitymanager_t *manager, entity_t *e)
{
    assert(manager);
    assert(e);

    //appending to entities list
    list_append(&manager->entities, e);

    // appending to map
    list_t *entitylist = entitymanager_get_all_entities_by_tag(manager, e->tag);
    list_append(entitylist, e);


    manager->total_entities++;

    return e;
}


void entitymanager_update(entitymanager_t *manager)
{
    assert(manager);


    // Remove dead entities from entities list and entitymap
    list_t *map = &manager->entitymap;
    list_t *entities = &manager->entities;
    for (u64 i = 0; i < entities->len; i++)
    {
        entity_t *e = (entity_t *)list_get_element_by_index(entities, i);
        assert(e);
        if (!e->is_alive) {

            entity_type tag = e->tag;
            list_delete(entities, i);

            list_t *entitylist = entitymanager_get_all_entities_by_tag(manager, tag);
            list_delete(entitylist, i);

            entity_destroy(e);
        }
    }
}


void entitymanager_destroy(entitymanager_t *manager)
{
    assert(manager);

    // Deleting all entities from entities list
    list_t *entities = &manager->entities;
    for (u64 i = 0; i < entities->len; i++)
    {
        entity_t *e = (entity_t *)list_get_element_by_index(entities, i);
        entity_destroy(e);
    }
    list_destroy(entities);

    // Deleting entitymap
    list_t *map = &manager->entitymap;
    for (u64 i = 0; i < map->len; i++)
    {
        list_t *list = (list_t *)list_get_element_by_index(map, i);
        list_destroy(list);
    }
    list_destroy(map);

    manager->total_entities = 0;
}
#endif

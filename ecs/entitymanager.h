#pragma once
#include "./entity.h"
#include "../ds/queue.h"




typedef struct entitymanager_t entitymanager_t;



entitymanager_t     entitymanager_init(const u64 total_entity_types);

entity_t *          entitymanager_add_entity(entitymanager_t *manager, entity_type tag);
void                entitymanager_update(entitymanager_t *manager);

void                entitymanager_destroy(entitymanager_t *manager);



#define             entitymanager_get_total_entities(PMANAGER)              (PMANAGER)->entities.len
#define             entitymanager_get_all_entities(PMANAGER)                &(PMANAGER)->entities
#define             entitymanager_get_all_entities_by_tag(PMANAGER, TAG)    (list_t *)list_get_element_by_index(&(PMANAGER)->entitymap, (TAG))





#ifndef IGNORE_ENTITY_MANAGER_IMPLEMENTATION

#define MAX_ENTITIES_ALLOWED_TO_BE_CREATED_PER_FRAME 100

struct entitymanager_t {

    //An list of all entities
    list_t      entities;

    //An list of list of entites differentiated by types
    list_t      entitymap; 

    // A queue of newly created entities ready to be added (solution to fix iterator invalidation)
    queue_t     __newly_added_entities;

};


entitymanager_t entitymanager_init(const u64 total_entity_types)
{
    assert(total_entity_types > 0);

    list_t map = list_init(total_entity_types, list_t );
    for (int i = 0; i < total_entity_types; i++)
    {
        list_t tmp = list_init(4, entity_t *);
        list_append(&map, tmp);
    }

    return (entitymanager_t ) {
        .entities = list_init(8, entity_t *),
        .entitymap = map,
        .__newly_added_entities = queue_init(MAX_ENTITIES_ALLOWED_TO_BE_CREATED_PER_FRAME, entity_t *)
    };
}

entity_t * entitymanager_add_entity(entitymanager_t *manager, entity_type tag)
{
    assert(manager);

    entity_t *e = __entity_init(tag);

    queue_t *queue = &manager->__newly_added_entities;
    queue_put(queue, e);

    return e;
}


void entitymanager_update(entitymanager_t *manager)
{
    assert(manager);

    // Remove dead entities from entities list and entitymap
    list_t *entities    = &manager->entities;
    for (u64 i = 0; i < entities->len; i++)
    {
        entity_t *e = (entity_t *)list_get_element_by_index(entities, i);
        assert(e);
        if (!e->is_alive) {

            entity_type tag = e->tag;
            list_delete(entities, i);

            list_t *entitylist = entitymanager_get_all_entities_by_tag(manager, tag);
            list_delete(entitylist, i);

            __entity_destroy(e);

        }
        e = NULL;
    }

    // Adding newly created entities into the entities list and map
    queue_t *queue = &manager->__newly_added_entities;
    while (!queue_is_empty(queue))
    {
        entity_t *e = (entity_t *)queue_get(queue);

        //appending to entities list
        list_append(&manager->entities, e);

        // appending to map
        list_t *entitymap = entitymanager_get_all_entities_by_tag(manager, e->tag);
        list_append(entitymap, e);
    }

}


void entitymanager_destroy(entitymanager_t *manager)
{
    assert(manager);

    // Deleting the queue
    assert(queue_is_empty(&manager->__newly_added_entities));
    queue_destroy(&manager->__newly_added_entities);

    // Deleting all entities from entities list
    list_t *entities = &manager->entities;
    for (u64 i = 0; i < entities->len; i++)
    {
        entity_t *e = (entity_t *)list_get_element_by_index(entities, i);
        __entity_destroy(e);
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

}
#endif

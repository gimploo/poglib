#pragma once
#include "./entity.h"

/*=============================================================================
                            - ENTITY MANAGER -
=============================================================================*/

typedef struct entitymanager_t {

    //An list of all entities
    list_t      entities;

    //An list of list of entites differentiated by types
    list_t      entitymap; 

    // A queue of newly created entities ready to be added (solution to fix iterator invalidation)
    queue_t     __newly_added_entities;

} entitymanager_t ;

entitymanager_t     entitymanager_init(const u64 total_entity_types);
#define             entitymanager_add_entity(PMANAGER, TAG)                     __impl_entitymanager_add_entity((PMANAGER), (TAG), #TAG)
void                entitymanager_update(entitymanager_t *manager);

#define             entitymanager_get_total_entities(PMANAGER)                  (PMANAGER)->entities.len
#define             entitymanager_get_entities_by_tag(PMANAGER, TAG)            (list_t *)list_get_value(&(PMANAGER)->entitymap, (TAG))

void                entitymanager_destroy(entitymanager_t *manager);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION 
-----------------------------------------------------------------------------*/

#ifndef IGNORE_ENTITY_MANAGER_IMPLEMENTATION

#define MAX_ENTITIES_ALLOWED_TO_BE_CREATED_PER_FRAME 100

entitymanager_t entitymanager_init(const u64 total_entity_types)
{
    assert(total_entity_types > 0);

    list_t map = list_init(list_t );
    for (u64 i = 0; i < total_entity_types; i++)
    {
        list_t tmp = list_init(entity_t *);
        list_append(&map, tmp);
    }

    return (entitymanager_t ) {
        .entities               = list_init(entity_t *),
        .entitymap              = map,
        .__newly_added_entities = queue_init(
                                    MAX_ENTITIES_ALLOWED_TO_BE_CREATED_PER_FRAME, 
                                    entity_t *)
    };
}

entity_t * __impl_entitymanager_add_entity(entitymanager_t *manager, entity_enumtype tag, const char *tagname)
{
    assert(manager);

    entity_t *e = __entity_init(tagname, tag);
    assert(e);

    queue_t *queue = &manager->__newly_added_entities;
    assert(queue);

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
        entity_t *e = (entity_t *)list_get_value(entities, i);
        assert(e);

        if (e->is_alive) continue;

        list_t *entitylist = entitymanager_get_entities_by_tag(manager, e->tag);
        assert(entitylist);

        for (u64 j = 0; j < entitylist->len; j++)
        {
            entity_t *tmp = (entity_t *)list_get_value(entitylist, j);
            assert(tmp);

            if (tmp->is_alive) continue;

            list_delete(entitylist, j);
            j = 0;
            tmp = NULL;
        }

        list_delete(entities, i);
        i = 0;

        __entity_free(e);
    }

    // Adding newly created entities into the entities list and map
    queue_t *queue = &manager->__newly_added_entities;
    assert(queue);

    while (!queue_is_empty(queue))
    {
        entity_t *e = (entity_t *)queue_get(queue);
        assert(e);

        //appending to entities list
        list_append(&manager->entities, e);

        // appending to map
        list_t *entitymap = entitymanager_get_entities_by_tag(manager, e->tag);
        list_append(entitymap, e);
    }

}


void entitymanager_destroy(entitymanager_t *manager)
{
    assert(manager);

    // Deleting the queue
    if(!queue_is_empty(&manager->__newly_added_entities))
    {
        entity_t *e = (entity_t *)queue_get(&manager->__newly_added_entities);
        assert(e);

        e->is_alive = false;
        __entity_free(e);
    }
    queue_destroy(&manager->__newly_added_entities);

    // Deleting all entities from entities list
    list_t *entities = &manager->entities;
    for (u64 i = 0; i < entities->len; i++)
    {
        entity_t *e = (entity_t *)list_get_value(entities, i);
        assert(e);

        e->is_alive = false;
        __entity_free(e);
    }
    list_destroy(entities);

    // Deleting entitymap
    list_t *map = &manager->entitymap;
    for (u64 i = 0; i < map->len; i++)
    {
        list_t *list = (list_t *)list_get_value(map, i);
        assert(list);

        list_destroy(list);
    }
    list_destroy(map);

}
#endif

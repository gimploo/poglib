#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/gfx/gl/types.h>
#include "./components.h"

/*============================================================================
                            - ENTITY -
============================================================================*/

typedef u32 entity_enumtype;

typedef struct entity_t {

    // Members
    const char              *label;
    const u64               *id;
    const entity_enumtype   tag; 
    bool                    is_alive;

    // dynamic list that holds pointers to components
    list_t                  components;

    // index buffer for grabbing components of the list
    i64                     __indices[ECT_COUNT];

} entity_t ;


#define         entity_destroy(PENTITY)                                 (PENTITY)->is_alive = false
#define         entity_add_component(PENTITY, COMPONENT, TYPE)          __impl_entity_add_component((PENTITY), &(COMPONENT), sizeof(COMPONENT), ECT_type(TYPE))
#define         entity_get_component(PENTITY, TYPE)                     __impl_entity_get_component((PENTITY), ECT_type(TYPE))
#define         entity_has_component(PENTITY, TYPE)                     ((PENTITY)->__indices[ECT_type(TYPE)] == -1 ? false : true)

/*----------------------------------------------------------------------------
                            IMPLEMENTATION
----------------------------------------------------------------------------*/

#ifndef IGNORE_ENTITY_IMPLEMENTATION

#define ECT_type(TYPE) ECT_ ## TYPE

void __entity_free(entity_t *e)
{
    assert(e);
    assert(e->is_alive == false);

    // Freeing the component list of all its component before destroying the list
    list_t *cmps = &e->components;
    list_destroy(cmps);
    cmps = NULL;

    // Zeroing in the indices buffer
    for (u32 i = 0; i < ARRAY_LEN(e->__indices); i++) e->__indices[i] = 0;

    e->id = NULL;

#ifdef DEBUG
    printf("[!] ENTITY (%s) DELETED\n", e->label);
#endif

    // Freeing the entity itself 
    free(e);
    e = NULL;
}

entity_t * __entity_init(const char *label, const entity_enumtype tag)
{
    entity_t *e = (entity_t *)calloc(1, sizeof(entity_t));
    assert(e);

    entity_t tmp = (entity_t ){

        .label = label,
        .id = (u64 *)e,
        .tag = tag,
        .is_alive = true,
        .components = list_init(entitycomponent_t ),
    };

    memcpy(e, &tmp, sizeof(tmp));

    for (u32 i = 0; i < ARRAY_LEN(e->__indices); i++) e->__indices[i] = -1;

#ifdef DEBUG
    printf("[!] ENTITY (%s) CREATED\n", e->label);
#endif

    return e;
}

void __impl_entity_add_component(entity_t *e, const void *ecmp_ref, const u64 ecmp_size, entitycomponent_type type)
{
    assert(e);
    assert(ecmp_ref);
    if(ecmp_size == 8) eprint("this component type is 8 bytes in size, add extra padding to the struct");

    // Wrapping the component into a entitycomponent_t type for better safety
    entitycomponent_t ec = (entitycomponent_t ) {
        .type = type,
    };
    memcpy(&ec.cmp, ecmp_ref, ecmp_size);

    // Appending the wrapper to the entity components list
    list_t *list = &e->components;
    assert(list);
    list_append(list, ec);

    // Updating the indices buffer
    e->__indices[type] = list->len - 1;
}


const void * __impl_entity_get_component(const entity_t *e, const entitycomponent_type type)
{
    assert(e);

    // No component found
    u64 index = e->__indices[type];
    if ((i64)index == -1) return NULL;

    entitycomponent_t *ec = (entitycomponent_t *)list_get_value(&e->components, index);
    if(!ec) eprint("entity %s\n", e->label);

    return &ec->cmp;
}

#endif

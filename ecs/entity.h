#pragma once
#include "../basic.h"
#include "../math/la.h"
#include "../ds/hashtable.h"
#include "../ds/list.h"
#include "../simple/gl/types.h"
#include "./components.h"




typedef struct entity_t entity_t ;
typedef u64 entity_type;




// API CALLS
#define         entity_destroy(PENTITY)                                    (PENTITY)->is_alive = false
#define         entity_add_component(PENTITY, PCOMPONENT, TYPE)          __impl_entity_add_component((PENTITY), (PCOMPONENT), ECT_type(TYPE))
#define         entity_get_component(PENTITY, TYPE)                     __impl_entity_get_component((PENTITY), ECT_type(TYPE))
#define         entity_has_component(PENTITY, TYPE)                     ((PENTITY)->__indices[ECT_type(TYPE)] == -1 ? false : true)

//NOTE: These functions can only be called by the entity manager and not alone
entity_t *      __entity_init(const char *tagname, entity_type tag);
void            __entity_destroy(entity_t *e);








#ifndef IGNORE_ENTITY_IMPLEMENTATION



#define ECT_type(TYPE) ECT_ ## TYPE

struct entity_t {

    // Members
    const char  *label;
    u64         *id;
    entity_type tag; 
    bool        is_alive;

    // dynamic list that holds pointers to components
    list_t      components;

    // index buffer for grabbing components of the list
    i64        __indices[ECT_COUNT];
};


struct entitycomponent_t {

    entitycomponent_type    type;
    void                    *cmp;
};


void __entity_destroy(entity_t *e)
{
    assert(e);
    assert(e->is_alive == false);

    // Freeing the component list of all its component before destroying the list
    list_t *cmps = &e->components;
    for (u64 i = 0; i < cmps->len; i++)
    {
        entitycomponent_t *ec = (entitycomponent_t *)list_get_element_by_index(cmps, i);
        assert(ec);

        switch(ec->type)
        {
            case ECT_c_shader_t: {

                c_shader_t *tmp = (c_shader_t *)ec->cmp;
                glshader_destroy(&tmp->glshader);

            } break;

            case ECT_c_texture2d_t: {

                c_texture2d_t *tmp = (c_texture2d_t *)ec->cmp; 
                gltexture2d_destroy(&tmp->gltexture2d);

            } break;
            
            default: ;
        }

        void *cmp = ec->cmp; 
        assert(cmp);
        free(cmp);

        cmp = NULL;
        ec = NULL;
    }
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

entity_t * __entity_init(const char *label, entity_type tag)
{
    entity_t *e = (entity_t *)calloc(1, sizeof(entity_t));
    assert(e);

    *e = (entity_t ) {
        .label = label,
        .id = (u64 *)e,
        .tag = tag,
        .is_alive = true,
        .components = list_init(4, entitycomponent_t ),
    };

    for (u32 i = 0; i < ARRAY_LEN(e->__indices); i++) e->__indices[i] = -1;

#ifdef DEBUG
    printf("[!] ENTITY (%s) CREATED\n", e->label);
#endif

    return e;
}

void __impl_entity_add_component(entity_t *e, void * ecmp_ref, entitycomponent_type type)
{
    assert(e);
    assert(ecmp_ref);

    // Wrapping the component into a entitycomponent_t type for better safety
    entitycomponent_t ec = (entitycomponent_t ) {
        .type = type,
        .cmp = ecmp_ref
    };

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

    u64 index = e->__indices[type];

    // No component found
    if ((i64)index == -1) return NULL;


    entitycomponent_t *ec = (entitycomponent_t *)list_get_element_by_index(&e->components, index);
    if(!ec) eprint("entity %s\n", e->label);
    if(!ec->cmp) eprint("entity %s\n", e->label);

    return ec->cmp;
}

#endif

#pragma once
#include <poglib/basic.h>
#include <poglib/ecs/component/types.h>

/* -------------------------------- ENTITY -------------------------------------------- */

#define MAX_ENTITY_COUNT (1 * MB)

typedef struct ecs_entitymanager_t ecs_entitymanager_t;
struct ecs_entitymanager_t {
    slot_t      entities;
    hashtable_t lookup;
};


/* ------------------------------- COMPONENT ------------------------------------------ */

typedef struct ecs_query_entitycmps_t ecs_query_entitycmps_t;
struct ecs_query_entitycmps_t{
    //NOTE: each component result index is cmp_idx (+1 increment) and 
    //not the bitmask index (power of 2)
    u32 hit_count;
    void *cmps[ECS_CMP_COUNT];
};

typedef struct ecs_componentmanager_t ecs_componentmanager_t;
struct ecs_componentmanager_t {

    slot_t componentpool_slots;                             //NOTE: this is going to hold all data of component type 
                                                            //packed together in a single array for a type, each type will 
                                                            //have its own slot index

    slot_t entity2component_lookup_slots;                   //NOTE: list of entityId to component index in packedcomponent_buffers 
                                                            //individual lookup tables
};


/* --------------------------------- SYSTEM ----------------------------------------- */

typedef void (*ecs_system_callback)(slot_t * const componentpool, const ecs_componentmanager_t * const cmp_manager);

typedef struct ecs_systemmanager_t ecs_systemmanager_t;
struct ecs_systemmanager_t {
    slot_t systems;
};


/* --------------------------------- ECS -------------------------------------------- */

typedef struct ecs_t ecs_t; 
struct ecs_t {

    arena_t arena;
    struct {
        ecs_entitymanager_t         entitymanager;
        ecs_componentmanager_t      componentmanager;
        ecs_systemmanager_t         systemmanager;
    } managers;

    struct {
        u32 entity_generator_counter;
    } internal;
};



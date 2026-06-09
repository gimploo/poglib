#pragma once
#include "poglib/ecs/component/colliderbatchqueue.h"
#include "poglib/util/glcamera.h"
#include <poglib/basic.h>
#include <poglib/ecs/component/types.h>


/* -------------------------------- ENTITY -------------------------------------------- */

#define ECS_ENTITY_INVALID_ID   0
#define ECS_ENTITY_MAX_COUNT    (1 * MB)

typedef struct ecs_entitymanager_t ecs_entitymanager_t;
struct ecs_entitymanager_t {
    slot_t      entities;
    hashtable_t lookup;
};


/* ------------------------------- COMPONENT ------------------------------------------ */

typedef struct ecs_componentmanager_t ecs_componentmanager_t;
typedef struct ecs_component_entry_t ecs_component_entry_t;
typedef struct ecs_entity_query_t ecs_entity_query_t;

struct ecs_entity_query_t {
    void *entity_cmp_data[ECS_CMP_COUNT];
};

struct ecs_componentmanager_t {

    slot_t componentpool_slots;                             //NOTE: this is going to hold all data of component type 
                                                            //packed together in a single array for a type, each type will 
                                                            //have its own slot index

    hashtable_t entity2components_lookup;                   //NOTE: tracks each index of an entity's component in the component 
                                                            //pool together packed into an array of `ECS_CMP_COUNT` size
    struct {
        colliderbatchqueue_t colliderbatch;
    } internal;
};

struct ecs_component_entry_t {
    ecs_component_type type;
    u32 entity_id;
    void *data;
};


/* --------------------------------- SYSTEM ----------------------------------------- */

#define ECS_SYSTEM_MAX_COUNT 10
#define ECS_SYSTEM_CALLBACK_MAX_ARG_COUNT 5

typedef struct ecs_system_t ecs_system_t; 
typedef struct ecs_systemmanager_t ecs_systemmanager_t;
typedef struct ecs_system_ctx_t ecs_system_ctx_t;

typedef void (*ecs_system_callback)(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx);

struct ecs_system_ctx_t {
    glcamera_t *active_camera;
    commandqueue_t *active_commandqueue;
};

struct ecs_system_t {
    ecs_system_callback callback;
};

struct ecs_systemmanager_t {
    u8 count;
    ecs_system_t systems[ECS_SYSTEM_MAX_COUNT];
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
        glcamera_t *active_camera;
        commandqueue_t *active_commandqueue;
    } internal;
};



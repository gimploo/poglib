#pragma once
#include "poglib/basic/common.h"
#include "poglib/gfx/glrenderer3d.h"
#include "poglib/math/la.h"
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/external/joltc/include/joltc.h>
#include <threads.h>

//TODO: see if whether the system needs its own arena.

typedef struct {
    void *entity_a;
    void *entity_b;
    JPH_BodyID id_a;
    JPH_BodyID id_b;
    //TODO: Add manifold data here if the scene needs the contact point/normal
    vec3 contact_point; 
} physics_sys_jolt_collision_event_t;

typedef struct {
    physics_sys_jolt_collision_event_t *events;
    u32 count;
    u32 capacity;
} physics_sys_jolt_event_queue_t;

typedef struct {
    JobSystemThreadPoolConfig config;
    JPH_JobSystem *jobsystem;
    JPH_ObjectLayerPairFilter *objectlayerpairfilter_table;
    JPH_BroadPhaseLayerInterface *broadphaselayerinterface_table;
    JPH_PhysicsSystemSettings systemsettings;
    JPH_ObjectVsBroadPhaseLayerFilter *objectvsbroadphaselayerfilter;
    JPH_PhysicsSystem* physics_system;
    JPH_BodyInterface* bodyinterface;
    struct {
        bool rules_configured;
        struct {
            physics_sys_jolt_event_queue_t buffer1;
            physics_sys_jolt_event_queue_t buffer2;
            physics_sys_jolt_event_queue_t *write_queue;
            physics_sys_jolt_event_queue_t *read_queue;
            mtx_t swap_mutex;
        } double_buffer_event_queue;
    } internal;
} physics_sys_jolt_t;

global physics_sys_jolt_t *global_physics_sys_jolt_instance = NULL;

#define MAX_COLLISION_INTERACTABILITY_ENTRIES       32 

typedef enum {
    PHY_BP_STATIC = 0,
    PHY_BP_DYNAMIC = 1,
    PHY_BP_SENSORS = 2,
    PHY_BP_DEBRIS = 3,
    PHY_BP_COUNT
} broadphase_type;

typedef struct {
    u16 objectlayer_type;
    u16 broadphase_type;
} collision_objectlayer_broadphase_config_t;

typedef struct {
    u16 count;
    collision_objectlayer_broadphase_config_t data[MAX_COLLISION_INTERACTABILITY_ENTRIES][2];
} physics_sys_jolt_rules_config_t;


physics_sys_jolt_t *                physics_sys_jolt_init(arena_t * const arena);
void                                physics_sys_jolt_set_interaction_rules(
                                        physics_sys_jolt_t * const self, 
                                        const physics_sys_jolt_rules_config_t config
                                    );
void                                physics_sys_jolt_start_simulation(physics_sys_jolt_t *self);

void                                physics_sys_jolt_update(physics_sys_jolt_t *self, const f32 dt);
physics_sys_jolt_event_queue_t *    physics_sys_get_collision_event_queue(const physics_sys_jolt_t * const self);

void                                physics_sys_jolt_destroy(physics_sys_jolt_t *self);

void                                physics_sys_jolt_character_destroy(JPH_CharacterVirtual *character);
void                                physics_sys_jolt_body_destroy(JPH_BodyID body_id);


#ifndef IGNORE_JOLT_WRAPPER_IMPLEMENTATION

void physics_sys_jolt__internal_tracefunc(const char* message)
{
    fprintf(stderr, "%s", message);
}

bool physics_sys_jolt__internal_assertfailurefunc(const char* expression, const char* message, const char* file, uint32_t line)
{
    fprintf(stderr, "Expression = %s\nMessage = %s", expression, message);
    fprintf(stderr, "Failed at line number %i in file %s\n", line, file);
    eprint("Failure in jolt");
    return false;
}

physics_sys_jolt_t * physics_sys_jolt_init(arena_t * const arena)
{
    ASSERT(global_physics_sys_jolt_instance == NULL);
    ASSERT(arena);
    physics_sys_jolt_t output = {
        .internal = {
            .double_buffer_event_queue = {
                .buffer1 = {
                    .capacity = 4000,
                    .events = arena_reserve(arena, sizeof(physics_sys_jolt_collision_event_t) * 4000),
                    .count = 0,
                },
                .buffer2 = {
                    .capacity = 4000,
                    .events = arena_reserve(arena, sizeof(physics_sys_jolt_collision_event_t) * 4000),
                    .count = 0,
                },
            },
            .rules_configured = false
        },
    };

    if (!JPH_Init()) {
        eprint("jolt failed to initalize");
    }

    if (mtx_init(&output.internal.double_buffer_event_queue.swap_mutex, mtx_plain) != thrd_success) {
        eprint("Failed to initialize C11 mutex for physics queue");
    }

    JPH_SetTraceHandler(physics_sys_jolt__internal_tracefunc);
    JPH_SetAssertFailureHandler(physics_sys_jolt__internal_assertfailurefunc);

    output.jobsystem = JPH_JobSystemThreadPool_Create(NULL);

    //NOTE: not used in example why ?
    //JPH_JobSystemCallback_Create(const JPH_JobSystemConfig* config);

    global_physics_sys_jolt_instance = mem_init(&output, sizeof(physics_sys_jolt_t));

    global_physics_sys_jolt_instance->internal.double_buffer_event_queue.read_queue = 
        &global_physics_sys_jolt_instance->internal.double_buffer_event_queue.buffer2;

    global_physics_sys_jolt_instance->internal.double_buffer_event_queue.write_queue = 
        &global_physics_sys_jolt_instance->internal.double_buffer_event_queue.buffer1;

    return global_physics_sys_jolt_instance;
}

void physics_sys_jolt_set_interaction_rules(physics_sys_jolt_t * const self, const physics_sys_jolt_rules_config_t config)
{
    ASSERT(config.count < MAX_COLLISION_INTERACTABILITY_ENTRIES);

    JPH_ObjectLayerPairFilter* objectLayerPairFilterTable = JPH_ObjectLayerPairFilterTable_Create(config.count * 2);

    u8 max_objectlayer_type = 0;
    u8 max_broadphase_type = 0;
    for (u32 i = 0; i < config.count; i++) {
        for (u8 j = 0; j < 2; j++) {
            if (config.data[i][j].objectlayer_type > max_objectlayer_type)
                max_objectlayer_type = config.data[i][j].objectlayer_type;
            if (config.data[i][j].broadphase_type > max_broadphase_type)
                max_broadphase_type = config.data[i][j].broadphase_type;
        }
    }

    struct {
        bool is_occupied;
        collision_objectlayer_broadphase_config_t config;
    } objectlayer_configs[MAX_COLLISION_INTERACTABILITY_ENTRIES] = {0};
    u16 objectlayer_count = 0;

    bool broadphase_types[MAX_COLLISION_INTERACTABILITY_ENTRIES] = {0};
    u16 broadphase_count = 0;

    for (u32 index = 0; index < config.count; index++)
    {
        JPH_ObjectLayerPairFilterTable_EnableCollision(
                objectLayerPairFilterTable, 
                config.data[index][0].objectlayer_type, 
                config.data[index][1].objectlayer_type);

        if (config.data[index][0].objectlayer_type != config.data[index][1].objectlayer_type)
            JPH_ObjectLayerPairFilterTable_EnableCollision(
                    objectLayerPairFilterTable, 
                    config.data[index][1].objectlayer_type, 
                    config.data[index][0].objectlayer_type);

        for (u8 objectlayer_config_index = 0; objectlayer_config_index < 2; objectlayer_config_index++)
        {
            const u8 config_object_type = config.data[index][objectlayer_config_index].objectlayer_type;
            const u8 config_broadphase_type = config.data[index][objectlayer_config_index].broadphase_type;
            const bool same_broadphase = objectlayer_configs[config_object_type].config.broadphase_type == config_broadphase_type;

            if (objectlayer_configs[config_object_type].is_occupied && same_broadphase)
                continue;

            if (objectlayer_configs[config_object_type].is_occupied && !same_broadphase)
                eprint("Broadphase mismatch - check the registeration again");

            objectlayer_configs[config_object_type].is_occupied = true;
            objectlayer_configs[config_object_type].config = config.data[index][objectlayer_config_index];
            objectlayer_count++;

            if (broadphase_types[config_broadphase_type])
                continue;

            broadphase_types[config_broadphase_type] = true;
            broadphase_count++;
        }
    }

    JPH_BroadPhaseLayerInterface* router = JPH_BroadPhaseLayerInterfaceTable_Create(objectlayer_count, broadphase_count);
    for (u8 index = 0; index < objectlayer_count; index++)
    {
        JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(
                router, 
                objectlayer_configs[index].config.objectlayer_type, 
                objectlayer_configs[index].config.broadphase_type);
    }


    JPH_ObjectVsBroadPhaseLayerFilter* objectVsBroadPhaseLayerFilter = JPH_ObjectVsBroadPhaseLayerFilterTable_Create(
        router,
        broadphase_count, 
        objectLayerPairFilterTable, 
        objectlayer_count);

    self->objectvsbroadphaselayerfilter = objectVsBroadPhaseLayerFilter;
    self->objectlayerpairfilter_table = objectLayerPairFilterTable;
    self->broadphaselayerinterface_table = router;
    self->internal.rules_configured = true;
}

void physics_sys_jolt_destroy(physics_sys_jolt_t *self)
{
    JPH_Shutdown();
    JPH_JobSystem_Destroy(self->jobsystem);

    mem_free(global_physics_sys_jolt_instance, sizeof(physics_sys_jolt_t));
    global_physics_sys_jolt_instance = NULL;

    mtx_destroy(&self->internal.double_buffer_event_queue.swap_mutex);
}

void physics_sys_jolt__internal_register_contact_listener(physics_sys_jolt_t * const self, const JPH_ContactListener_Procs listeners)
{
    JPH_ContactListener_SetProcs(&listeners);
    JPH_ContactListener* listener = JPH_ContactListener_Create(NULL);
    JPH_PhysicsSystem_SetContactListener(self->physics_system, listener);
}

void physics_sys_jolt__internal_on_contact_added(void* userData, const JPH_Body* b1, const JPH_Body* b2, const JPH_ContactManifold* m, JPH_ContactSettings* s) 
{
    physics_sys_jolt_t* sys = (physics_sys_jolt_t*)userData;

    mtx_lock(&sys->internal.double_buffer_event_queue.swap_mutex);
    {
        if ((sys->internal.double_buffer_event_queue.write_queue->count + 1) == sys->internal.double_buffer_event_queue.write_queue->capacity) {
            logging("Collision Event Queue failed to add an event due to size constraint");
        } else {
            physics_sys_jolt_collision_event_t* ev = &sys->internal.double_buffer_event_queue.write_queue->events[sys->internal.double_buffer_event_queue.write_queue->count++];
            ev->entity_a = (void *)JPH_Body_GetUserData((JPH_Body *)b1);
            ev->entity_b = (void *)JPH_Body_GetUserData((JPH_Body *)b2);
            ev->id_a = JPH_Body_GetID(b1);
            ev->id_b = JPH_Body_GetID(b2);
            //TODO: Extract manifold point if needed...
        } 
    }
    mtx_unlock(&sys->internal.double_buffer_event_queue.swap_mutex);
}

void physics_sys_jolt_start_simulation(physics_sys_jolt_t *self)
{
    if (!self->internal.rules_configured)
        eprint("physics system requires rules to be configured for starting simulation");

	self->systemsettings = (JPH_PhysicsSystemSettings){0};
	self->systemsettings.maxBodies = 65536;
	self->systemsettings.numBodyMutexes = 0;
	self->systemsettings.maxBodyPairs = 65536;
	self->systemsettings.maxContactConstraints = 65536;

	self->systemsettings.objectVsBroadPhaseLayerFilter  = self->objectvsbroadphaselayerfilter;
	self->systemsettings.broadPhaseLayerInterface       = self->broadphaselayerinterface_table;
	self->systemsettings.objectLayerPairFilter          = self->objectlayerpairfilter_table;

	self->physics_system    = JPH_PhysicsSystem_Create(&self->systemsettings);
	self->bodyinterface     = JPH_PhysicsSystem_GetBodyInterface(self->physics_system);

    physics_sys_jolt__internal_register_contact_listener(
        self, 
        (JPH_ContactListener_Procs ) {
            .OnContactAdded = physics_sys_jolt__internal_on_contact_added,
            .OnContactPersisted = NULL,
            .OnContactRemoved = NULL,
            .OnContactValidate = NULL
        }
    );
}

void physics_sys_jolt_update(physics_sys_jolt_t *self, const f32 dt) {

    JPH_PhysicsSystem_Update(self->physics_system, dt, 1, self->jobsystem);

    //NOTE: no need to lock to swap the buffer as per gemini since no contention 
    //happens here as its post `JPH_PhysicsSystem_Update` call
    physics_sys_jolt_event_queue_t *const temp = self->internal.double_buffer_event_queue.read_queue;
    self->internal.double_buffer_event_queue.read_queue = self->internal.double_buffer_event_queue.write_queue;
    self->internal.double_buffer_event_queue.write_queue = temp;
    self->internal.double_buffer_event_queue.write_queue->count = 0;
}

physics_sys_jolt_event_queue_t * physics_sys_get_collision_event_queue(const physics_sys_jolt_t *const self)
{
    return self->internal.double_buffer_event_queue.read_queue;
}

void physics_sys_jolt_character_destroy(JPH_CharacterVirtual *const character)
{
    JPH_CharacterBase_Destroy((JPH_CharacterBase *)character);
}

void physics_sys_jolt_body_destroy(const JPH_BodyID body_id)
{
    JPH_BodyInterface_RemoveAndDestroyBody(
        global_physics_sys_jolt_instance->bodyinterface,
        body_id);
}


JPH_RayCastResult physics_sys_jolt_raycast(const vec3f_t ray_pos, const vec3f_t dir)
{
    ASSERT(global_physics_sys_jolt_instance);
    const JPH_NarrowPhaseQuery* npq = JPH_PhysicsSystem_GetNarrowPhaseQuery(global_physics_sys_jolt_instance->physics_system);
    JPH_RayCastResult hit = {0};
    if (JPH_NarrowPhaseQuery_CastRay(npq, (JPH_Vec3 *)&ray_pos, (JPH_Vec3 *)&dir, &hit, NULL, NULL, NULL)) {
        return hit;
    }
    return (JPH_RayCastResult){0};
}

#endif

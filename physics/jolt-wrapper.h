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

typedef struct {
    u16 objectlayer_type;
    u16 broadphase_type;
} collision_objectlayer_broadphase_config_t;

typedef struct {
    u16 count;
    collision_objectlayer_broadphase_config_t data[MAX_COLLISION_INTERACTABILITY_ENTRIES][2];
} physics_sys_jolt_rules_config_t;


physics_sys_jolt_t *    physics_sys_jolt_init(arena_t * const arena);
void                    physics_sys_jolt_set_interaction_rules(
                            physics_sys_jolt_t * const self, 
                            const physics_sys_jolt_rules_config_t config
                        );
void                    physics_sys_jolt_start_simulation(physics_sys_jolt_t *self);

u32 physics_sys_jolt_create_box(
                        physics_sys_jolt_t *self, 
                        const vec3f_t position, 
                        const vec3f_t half_extents, 
                        const JPH_MotionType motion_type, 
                        const JPH_ObjectLayer layer,
                        const JPH_Activation isActivationMode );

u32 physics_sys_jolt_create_capsule(
                        physics_sys_jolt_t *self, 
                        const vec3f_t position, 
                        const f32 halfHeightOfCylinder,
                        const f32 radius,
                        const JPH_MotionType motion_type, 
                        const JPH_ObjectLayer layer,
                        const JPH_Activation isActivationMode);

physics_sys_jolt_event_queue_t * physics_sys_get_collision_event_queue(const physics_sys_jolt_t * const self);

matrix4f_t              physics_sys_get_world_transform(physics_sys_jolt_t *self, JPH_BodyID body_id);
void                    physics_sys_jolt_update(physics_sys_jolt_t *self, const f32 dt);
void                    physics_sys_jolt_destroy(physics_sys_jolt_t *self);


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
                    .events = arena_reserve_array(arena, physics_sys_jolt_collision_event_t, 4000),
                    .count = 0,
                },
                .buffer2 = {
                    .capacity = 4000,
                    .events = arena_reserve_array(arena, physics_sys_jolt_collision_event_t, 4000),
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

	JPH_ObjectLayerPairFilter* objectLayerPairFilterTable = JPH_ObjectLayerPairFilterTable_Create(config.count);

    struct {
        u16 count;
        struct {
            bool is_occupied;
            collision_objectlayer_broadphase_config_t config;
        } data[sizeof(u16)];
    } objectlayer_configs  = {0};

    struct {
        u16 count;
        struct {
            bool is_occupied;
        } data[sizeof(u16)];
    } broadphase_types  = {0};

    for (u32 index = 0; index < config.count; index++)
    {
        JPH_ObjectLayerPairFilterTable_EnableCollision(
                objectLayerPairFilterTable, 
                config.data[index][0].objectlayer_type, 
                config.data[index][1].objectlayer_type);

        for (u8 objectlayer_config_index = 0; objectlayer_config_index < 2; objectlayer_config_index++)
        {
            const u8 object_type = config.data[index][objectlayer_config_index].objectlayer_type;

            if(objectlayer_configs.data[object_type].is_occupied)
                continue;

            objectlayer_configs.data[object_type].is_occupied = true;
            objectlayer_configs.data[object_type].config = config.data[index][objectlayer_config_index];
            objectlayer_configs.count++;

            const u8 broadphase_type = config.data[index][objectlayer_config_index].broadphase_type;
            if (broadphase_types.data[broadphase_type].is_occupied)
                continue;

            broadphase_types.data[broadphase_type].is_occupied = true;
            broadphase_types.count++;
        }
    }

    JPH_BroadPhaseLayerInterface* router = JPH_BroadPhaseLayerInterfaceTable_Create(objectlayer_configs.count, broadphase_types.count);
    for (u8 index = 0; index < objectlayer_configs.count; index++)
    {
        JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(
                router, 
                objectlayer_configs.data[index].config.objectlayer_type, 
                objectlayer_configs.data[index].config.broadphase_type);
    }


	JPH_ObjectVsBroadPhaseLayerFilter* objectVsBroadPhaseLayerFilter = JPH_ObjectVsBroadPhaseLayerFilterTable_Create(
            self->broadphaselayerinterface_table,
            broadphase_types.count, 
            objectLayerPairFilterTable, 
            objectlayer_configs.count);

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
	self->systemsettings.broadPhaseLayerInterface = self->broadphaselayerinterface_table;
	self->systemsettings.objectLayerPairFilter = self->objectlayerpairfilter_table;
	self->systemsettings.objectVsBroadPhaseLayerFilter = self->objectvsbroadphaselayerfilter;

	self->physics_system = JPH_PhysicsSystem_Create(&self->systemsettings);
	self->bodyinterface = JPH_PhysicsSystem_GetBodyInterface(self->physics_system);

    physics_sys_jolt__internal_register_contact_listener(self, (JPH_ContactListener_Procs ) {
            .OnContactAdded = physics_sys_jolt__internal_on_contact_added,
            .OnContactPersisted = NULL,
            .OnContactRemoved = NULL,
            .OnContactValidate = NULL
        });
}

// Example function to add to your wrapper
u32 physics_sys_jolt_create_box(
    physics_sys_jolt_t *self, 
    const vec3f_t position, 
    //NOTE: he distance from the center of a 3D box, shape, or collider 
    //to its outer edges or boundaries, representing half the total size in each dimension
    const vec3f_t half_extents, 
    const JPH_MotionType motion_type, 
    const JPH_ObjectLayer layer,
    //NOTE: 
    //inActivationMode: Determines the initial state of the body:
    //* JPH_Activation_Activate:        The body starts "awake" and is simulated immediately.
    //* JPH_Activation_DontActivate:    The body starts "asleep" and won't move until something hits it or you manually wake it up.
    const JPH_Activation isActivationMode 
) {
    // 1. Create the Shape directly
    // JPH_DEFAULT_CONVEX_RADIUS is defined as 0.05f in your header
    JPH_BoxShape* box_shape = JPH_BoxShape_Create(
            (JPH_Vec3*)&half_extents, 
            JPH_DEFAULT_CONVEX_RADIUS);

    // 2. Setup Body Creation Settings
    // Note: We use Create3 because it takes a JPH_Shape* directly
    JPH_BodyCreationSettings* body_settings = JPH_BodyCreationSettings_Create3(
        (JPH_Shape*)box_shape, 
        (JPH_Vec3*)&position, 
        &(JPH_Quat){ 0, 0, 0, 1 }, 
        motion_type, 
        layer
    );

    // 3. Add to system
    JPH_BodyID body_id = JPH_BodyInterface_CreateAndAddBody(
        self->bodyinterface, 
        body_settings, 
        isActivationMode
    );

    // 4. Cleanup temporary settings
    // The Body now has a reference to the shape, so we can destroy our local handles
    JPH_BodyCreationSettings_Destroy(body_settings);
    JPH_Shape_Destroy((JPH_Shape*)box_shape); 

    return body_id;
}

// Example function to add to your wrapper
u32 physics_sys_jolt_create_capsule(
    physics_sys_jolt_t *self, 
    const vec3f_t position, 
    const f32 halfHeightOfCylinder,
    const f32 radius, //INFO: not the convex radius!
    const JPH_MotionType motion_type, 
    const JPH_ObjectLayer layer,
    const JPH_Activation isActivationMode 
) {

    // 1. Create the Shape directly
    // JPH_DEFAULT_CONVEX_RADIUS is defined as 0.05f in your header
    JPH_CapsuleShape* capsule_shape = JPH_CapsuleShape_Create(
            halfHeightOfCylinder,
            radius);

    // 2. Setup Body Creation Settings
    // Note: We use Create3 because it takes a JPH_Shape* directly
    JPH_BodyCreationSettings* body_settings = JPH_BodyCreationSettings_Create3(
        (JPH_Shape*)capsule_shape, 
        (JPH_Vec3*)&position, 
        &(JPH_Quat){ 0, 0, 0, 1 }, 
        motion_type, 
        layer
    );

    // 3. Add to system
    JPH_BodyID body_id = JPH_BodyInterface_CreateAndAddBody(
        self->bodyinterface, 
        body_settings, 
        isActivationMode
    );

    // 4. Cleanup temporary settings
    // The Body now has a reference to the shape, so we can destroy our local handles
    JPH_BodyCreationSettings_Destroy(body_settings);
    JPH_Shape_Destroy((JPH_Shape*)capsule_shape); 

    return body_id;
}

void physics_sys_jolt_update(physics_sys_jolt_t *self, const f32 dt) {

    JPH_PhysicsSystem_Update(self->physics_system, dt, 1, self->jobsystem);

    //NOTE: no need to lock to swap the buffer as per gemini since no contention 
    //happens here as its post `JPH_PhysicsSystem_Update` call
    physics_sys_jolt_event_queue_t* temp = self->internal.double_buffer_event_queue.read_queue;
    self->internal.double_buffer_event_queue.read_queue = self->internal.double_buffer_event_queue.write_queue;
    self->internal.double_buffer_event_queue.write_queue = temp;
    self->internal.double_buffer_event_queue.write_queue->count = 0;
}

matrix4f_t physics_sys_get_world_transform(physics_sys_jolt_t *self, JPH_BodyID body_id)
{
    JPH_RMat4 matrix;
    JPH_BodyInterface_GetWorldTransform(
            self->bodyinterface, 
            body_id, 
            &matrix);

    matrix4f_t result = {0};
    memcpy(&result, &matrix, sizeof(matrix4f_t));
    return result;
}

physics_sys_jolt_event_queue_t * physics_sys_get_collision_event_queue(const physics_sys_jolt_t * const self)
{
    return self->internal.double_buffer_event_queue.read_queue;
}

#endif

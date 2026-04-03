#pragma once
#include <poglib/basic.h>
#include <poglib/external/joltc/include/joltc.h>

typedef struct {
    JobSystemThreadPoolConfig config;
    JPH_JobSystem *jobsystem;
    JPH_ObjectLayerPairFilter *objectlayerpairfilter_table;
    JPH_BroadPhaseLayerInterface *broadphaselayerinterface_table;
    JPH_PhysicsSystemSettings systemsettings;
    JPH_ObjectVsBroadPhaseLayerFilter *objectvsbroadphaselayerfilter;
    JPH_PhysicsSystem* physics_system;
    JPH_BodyInterface* bodyinterface;
} physics_sys_jolt_t;

#define MAX_COLLISION_INTERACTABILITY_ENTRIES       32 

typedef struct {
    u8 objectlayer_type;
    u8 broadphase_type;
} collision_object_broadphase_config_t;

typedef struct {
    u32 count;
    collision_object_broadphase_config_t data[MAX_COLLISION_INTERACTABILITY_ENTRIES][2];
} physcis_sys_jolt_rules_config_t;


physics_sys_jolt_t      physics_sys_jolt_init(void);
void                    physics_sys_jolt_configure_entity_interaction(
                            physics_sys_jolt_t *self, 
                            physcis_sys_jolt_rules_config_t config
                        );
void                    physics_sys_jolt_start_simulation(physics_sys_jolt_t *self);
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

physics_sys_jolt_t physics_sys_jolt_init(void)
{
    physics_sys_jolt_t output = {0};

    if (JPH_Init()) {
        eprint("jolt failed to initalize");
    }

    JPH_SetTraceHandler(physics_sys_jolt__internal_tracefunc);
    JPH_SetAssertFailureHandler(physics_sys_jolt__internal_assertfailurefunc);

    output.jobsystem = JPH_JobSystemThreadPool_Create(NULL);

    //NOTE: not used in example why ?
    //JPH_JobSystemCallback_Create(const JPH_JobSystemConfig* config);

}

void physics_sys_jolt_configure_entity_interaction(physics_sys_jolt_t *self, physcis_sys_jolt_rules_config_t config)
{
    ASSERT(config.count);
    ASSERT(config.count < MAX_COLLISION_INTERACTABILITY_ENTRIES);

	JPH_ObjectLayerPairFilter* objectLayerPairFilterTable = JPH_ObjectLayerPairFilterTable_Create(config.count);

    struct {
        u32 count;
        struct {
            bool is_occupied;
            collision_object_broadphase_config_t config;
        } data[sizeof(u8)];
    } objectlayer_configs  = {0};

    struct {
        u32 count;
        struct {
            bool is_occupied;
        } data[sizeof(u8)];
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
}

void physics_sys_jolt_destroy(physics_sys_jolt_t *self)
{
    JPH_Shutdown();
    JPH_JobSystem_Destroy(self->jobsystem);
}

void physics_sys_jolt_start_simulation(physics_sys_jolt_t *self)
{
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
}

#endif

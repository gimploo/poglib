#pragma once
#include "./ecs/entitymanager.h"
#include "./ecs/components.h"

//TODO: systems dont work anymore (very old code);
//#include "./ecs/systems.h"

typedef struct ecs_t {

    entitymanager_t entities;
    assetmanager_t assets;

    //TODO: Implement systems manager (since entites takes care of components we can have 
    //a separate manager for systems to make managing easier i think!)

} ecs_t;


ecs_t ecs_init(const u32 total_entity_types)
{
    return (ecs_t ){
        .entities = entitymanager_init(total_entity_types),
        .assets = assetmanager_init() 
    };
}

void ecs_update(ecs_t *s, const f32 dt)
{
    entitymanager_update(&s->entities);
}

void ecs_destroy(ecs_t *s)
{
    entitymanager_destroy(&s->entities);
    assetmanager_destroy(&s->assets);
}


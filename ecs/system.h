#pragma once


typedef struct ecs_system_t ecs_system_t;
struct ecs_system_t {
    void (*system_callback)(void);
};

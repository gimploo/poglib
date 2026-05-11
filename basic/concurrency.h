#pragma once
#include <threads.h>
#include <stdatomic.h>
#include "./dbg.h"
#include "./common.h"
#include "./arena.h"
#include "./ds/queue.h"
#include "poglib/basic/str.h"

//TODO: way to safely exit from active running threads.

#define __ASYNC_META_HEADER__\
    atomic_bool is_done;\
    thrd_t thrd_id;\

#define async(TYPE) struct {\
    __ASYNC_META_HEADER__\
    TYPE *data;\
}

typedef struct taskresponse_t taskresponse_t;
struct taskresponse_t {
    __ASYNC_META_HEADER__
    void *resource;
};

typedef struct taskpayload_t taskpayload_t;
struct taskpayload_t {
    struct {
        u32 count;
        union {
            str_t str;
            void *any;
        } arg[4];
    } args;
    struct {
        bool is_ready;
        arena_t arena;
    } storage;
};

typedef struct taskconfig_t taskconfig_t;
struct taskconfig_t {
    taskpayload_t       payload;
    taskresponse_t      *result_dest;
    void (*callback)(const taskpayload_t args, void *output_reserved_mem);
};

typedef struct bgtask_manager_t bgtask_manager_t;
struct bgtask_manager_t {
    queue_t tasks;
    arena_t arena;
};

taskresponse_t *    taskresponse(arena_t * const arena, const u64 response_size);
taskresponse_t *    task_response_empty(arena_t * const arena);
void                taskresponse_destroy(taskresponse_t *self);


bgtask_manager_t    bgtask_manager_init(void);
void                bgtask_manager_pass_task(bgtask_manager_t * const self, const taskconfig_t config);
void                bgtask_manager_run_all_tasks(bgtask_manager_t *self);
void                bgtask_manager_destroy(bgtask_manager_t *self);


#ifndef IGNORE_CONCURRENCY_IMPLEMENTATION

#define TOTAL_THREADS_AVAILABLE 4

typedef struct {
    taskconfig_t config;
    taskresponse_t *response_ref;
} bgtask__internal_t;

typedef struct {
    bgtask__internal_t task;
    arena_t *bgarena;
} thread__internal_payload_t;

taskresponse_t * taskresponse(arena_t * const arena, const u64 response_size)
{
    taskresponse_t *taskresponse = arena_reserve(arena, sizeof(taskresponse_t));
    taskresponse->resource = response_size > 0 ? arena_reserve(arena, response_size) : NULL;
    return taskresponse;
}

bgtask_manager_t bgtask_manager_init(void)
{
    return (bgtask_manager_t) {
        .tasks = queue_init(TOTAL_THREADS_AVAILABLE, bgtask__internal_t, NULL),
        .arena = arena_init(NULL, 8 * KB)
    };
}

void bgtask_manager_pass_task(bgtask_manager_t * const self, const taskconfig_t config)
{
    ASSERT(config.payload.args.count > 0);
    ASSERT(config.callback);

    config.result_dest->thrd_id = (thrd_t){0};

    const bgtask__internal_t task = (bgtask__internal_t){
        .config = config,
        .response_ref = config.result_dest,
    };
    queue_put(&self->tasks, task);
}


i32 bgtask__internal_thread_wrapper(void *thread_payload_data)
{
    thread__internal_payload_t *payload = thread_payload_data;

    payload->task.config.callback(
        payload->task.config.payload,
        payload->task.response_ref->resource
    );

    atomic_store_explicit(&payload->task.response_ref->is_done, true, memory_order_release);
    arena_giveback(payload->bgarena, payload, sizeof(thread__internal_payload_t));

    return 0;
}

void bgtask_manager_run_all_tasks(bgtask_manager_t *self)
{
    //TODO: maybe have this restrain to few threads only
    while(!queue_is_empty(&self->tasks)) {

        bgtask__internal_t task = {0};
        queue_get_in_buffer(&self->tasks, (buffer_t) { 
            .raw_data = (void *)&task, 
            .size = sizeof(task) 
        });

        thread__internal_payload_t *payload = arena_reserve(&self->arena, sizeof(thread__internal_payload_t));
        *payload = (thread__internal_payload_t){
            .task = task,
            .bgarena = &self->arena
        };

        if (thrd_create(&payload->task.response_ref->thrd_id, bgtask__internal_thread_wrapper, payload) != thrd_success) {
            eprint("Failed to generate thread");
        }
    }
}

void bgtask_manager_destroy(bgtask_manager_t *self)
{
    queue_destroy(&self->tasks);
    arena_destroy(&self->arena);
    memset(self, 0, sizeof(bgtask_manager_t));
}

void taskresponse_destroy(taskresponse_t *self) 
{
    taskresponse_t *obj = self;
#ifdef _WIN32
    thrd_exit(obj->thrd_id._Tid);
#else
    thrd_exit(obj->thrd_id);
#endif
}

#endif


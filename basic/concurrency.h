#pragma once
#include <threads.h>
#include <stdatomic.h>
#include "./dbg.h"
#include "./common.h"
#include "./arena.h"
#include "./ds/queue.h"
#include "poglib/basic/str.h"

#define __ASYNC_META_HEADER__\
    atomic_bool is_done;\
    thrd_t thrd_id;\

typedef struct {
    __ASYNC_META_HEADER__
    void *resource;
} taskresponse_t;

#define async(TYPE) struct {\
    __ASYNC_META_HEADER__\
    TYPE *data;\
}

taskresponse_t * task_response(arena_t * const arena, const u64 response_size)
{
    ASSERT(response_size);
    taskresponse_t *taskresponse = arena_reserve(arena, sizeof(taskresponse_t));
    taskresponse->resource = arena_reserve(arena, response_size);
    return taskresponse;
}

taskresponse_t * task_response_empty(arena_t * const arena)
{
    taskresponse_t *asyncobj = arena_reserve(arena, sizeof(taskresponse_t));
    asyncobj->resource = NULL;
    return asyncobj;
}

typedef struct {

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

} taskpayload_t;

int i = sizeof(taskresponse_t);

typedef struct {
    taskpayload_t payload;
    void (*callback)(const taskpayload_t args, void *output_reserved_mem);
} taskconfig_t;

typedef struct {
    taskconfig_t config;
    taskresponse_t *response_ref;
} bgtask__internal_t;

typedef struct {

    bgtask__internal_t task;
    arena_t *bgarena;

} thread__internal_payload_t;

typedef struct {

    queue_t tasks;
    arena_t arena;

} bgtask_manager_t;

#define TOTAL_THREADS_AVAILABLE 4

bgtask_manager_t bgtask_manager_init(void)
{
    return (bgtask_manager_t) {
        .tasks = queue_init(TOTAL_THREADS_AVAILABLE, bgtask__internal_t, NULL),
        .arena = arena_init(NULL, 8 * KB)
    };
}

void bgtask_manager_pass_task(
    bgtask_manager_t * const self, 
    const taskconfig_t config,
    taskresponse_t* const response_ref
) {
    ASSERT(response_ref);
    ASSERT(config.payload.args.count > 0);
    ASSERT(config.callback);

    response_ref->thrd_id = (thrd_t){0};

    const bgtask__internal_t task = (bgtask__internal_t){
        .config = config,
        .response_ref = response_ref,
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

void async_destroy(void *self) 
{
    taskresponse_t *obj = self;
#ifdef _WIN32
    thrd_exit(obj->thrd_id._Tid);
#else
    thrd_exit(obj->thrd_id);
#endif
}



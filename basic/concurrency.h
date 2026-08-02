#pragma once
#include <poglib/basic.h>
#include "./dbg.h"
#include "./common.h"
#include "./arena.h"
#include "./ds/queue.h"

//TODO: way to safely exit from active running threads.

#define TOTAL_THREADS_AVAILABLE 32

typedef struct bgtask_manager_t bgtask_manager_t;
bgtask_manager_t *global_bgtask_manager = NULL;

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

typedef struct taskparams_t taskparams_t;
struct taskparams_t {
    u8 count;
    union {
        str_t   str;
        u64     u64;
        arena_t *arena;
        void    *any;
    } arg[8];
};

typedef union taskstorage_t taskstorage_t;
union taskstorage_t {
    buffer_t buffer;
    arena_t *transient_arena;
};

typedef struct taskpayload_t taskrequest_t;
struct taskpayload_t {
    taskparams_t        params;
    taskstorage_t       storage;
    taskresponse_t      *response;
    void (*callback)(const taskparams_t args, taskstorage_t storage, taskresponse_t *const response);
    struct {
        taskparams_t params;
        void (*callback)(const taskparams_t params);
    } on_complete;
};

struct bgtask_manager_t {
    queue_t tasks;
};

taskresponse_t *    taskresponse(arena_t * const arena, const u64 response_size);
taskresponse_t *    task_response_empty(arena_t * const arena);
void                taskresponse_destroy(taskresponse_t *self);


bgtask_manager_t *  bgtask_manager_init(arena_t *const arena);
void                bgtask_manager_pass_task(bgtask_manager_t * const self, const taskrequest_t payload);
void                bgtask_manager_run_all_tasks(bgtask_manager_t *self);
void                bgtask_manager_destroy(bgtask_manager_t *self);


#ifndef IGNORE_CONCURRENCY_IMPLEMENTATION


taskresponse_t * taskresponse(arena_t *const arena, const u64 response_size)
{
    taskresponse_t *taskresponse = arena_reserve(arena, sizeof(taskresponse_t));
    taskresponse->resource = response_size > 0 ? arena_reserve(arena, response_size) : NULL;
    return taskresponse;
}

bgtask_manager_t * bgtask_manager_init(arena_t *const arena)
{
    ASSERT(!global_bgtask_manager);

    global_bgtask_manager = arena_store(
        arena,
        &(bgtask_manager_t) {
            .tasks = queue_init(TOTAL_THREADS_AVAILABLE, taskrequest_t, arena),
        }, 
        sizeof(bgtask_manager_t)
    );
    return global_bgtask_manager;
}

void bgtask_manager_pass_task(bgtask_manager_t * const self, const taskrequest_t request)
{
    ASSERT(request.params.count > 0);
    ASSERT(request.callback);

    request.response->thrd_id = (thrd_t){0};
    queue_put(&self->tasks, &request, sizeof(request));
}


i32 bgtask__internal_thread_wrapper(void *thread_payload_data)
{
    taskrequest_t *const payload = thread_payload_data;
    ASSERT(payload->response);
    ASSERT(payload->response->resource);

    payload->callback(
        payload->params,
        payload->storage,
        payload->response
    );

    atomic_store_explicit(&payload->response->is_done, true, memory_order_release);

    if (payload->on_complete.callback) {
        ASSERT(payload->on_complete.params.count);
        payload->on_complete.callback(payload->on_complete.params);
    }

    mem_free(payload, sizeof(*payload));

    return 0;
}

void bgtask_manager_run_all_tasks(bgtask_manager_t *const self)
{
    //TODO: maybe have this restrain to few threads only
    while(!queue_is_empty(&self->tasks)) {

        taskrequest_t *task = mem_init(NULL, sizeof(taskrequest_t));
        queue_get_in_buffer(&self->tasks, (buffer_t) { 
            .raw_data = (void *)task, 
            .size = sizeof(*task) 
        });
        if (thrd_create(&task->response->thrd_id, bgtask__internal_thread_wrapper, task) != thrd_success) {
            eprint("Failed to generate thread");
        }
    }
}

void bgtask_manager_destroy(bgtask_manager_t *self)
{
    ASSERT(global_bgtask_manager);

    queue_destroy(&self->tasks);
}

void taskresponse_destroy(taskresponse_t *self) 
{
    taskresponse_t *obj = self;
#ifdef _WIN32
    thrd_exit(obj->thrd_id._Tid);
#else
    thrd_exit((intptr_t)obj->thrd_id);
#endif
}

#endif


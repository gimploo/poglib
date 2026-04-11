#pragma once
#include <threads.h>
#include <stdatomic.h>
#include "./dbg.h"
#include "./common.h"
#include "./arena.h"
#include "./ds/queue.h"

#define __ASYNC_META_HEADER__\
    atomic_bool is_done;\
    struct {\
        thrd_t id;\
    } thrd;\

typedef struct {
    __ASYNC_META_HEADER__
    void *resource;
} async_object_t;

#define async(TYPE) struct {\
    __ASYNC_META_HEADER__\
    TYPE *data;\
}

typedef struct {
    u32 count;
    void *arg1;
    void *arg2;
    void *arg3;
    void *arg4;
} task_args_t;

typedef struct {
    struct {
        void *(*func_ptr_ret)(task_args_t args, arena_t *);
        void (*func_ptr_no_ret)(task_args_t args);
    };
    task_args_t arg;
    arena_t *arena;
} task_config_t;

typedef struct {
    task_config_t config;
    struct {
        async_object_t *async_obj;
        const void **res_addr;
    } meta;
} bgtask_t;

typedef struct {

    bgtask_t task;
    struct {
        arena_t *persitent;
        arena_t scratch;
    } arenas;

} thread_payload_t;

typedef struct {

    queue_t tasks;
    arena_t arena;

} bgtask_manager_t;

bgtask_manager_t bgtask_manager_init(void)
{
    return (bgtask_manager_t) {
        .tasks = queue_init(10, bgtask_t, NULL),
        .arena = arena_init(NULL, 2 * MB)
    };
}

void bgtask_manager_pass_task(
    bgtask_manager_t * const self, 
    const task_config_t config,
    void * const async_object_obj
) {
    async_object_t * const async_obj = async_object_obj;
    ASSERT(async_obj);

    async_obj->thrd.id = (thrd_t){0};

    if ((config.arena == NULL && config.func_ptr_ret != NULL)
        ||(config.arena != NULL && config.func_ptr_ret == NULL)) {
        eprint("Pass an arena when func_ptr_ret is set");
    }

    const bgtask_t task = (bgtask_t){
        .config = config,
        .meta = {
            .async_obj = async_obj,
            .res_addr = config.func_ptr_ret ? (const void **)&async_obj->resource : NULL
        }
    };
    queue_put(&self->tasks, task);
}


i32 bgtask__internal_thread_wrapper(void *data)
{
    thread_payload_t *payload = data;
    if (payload->arenas.persitent) {
        const void *output = payload->task.config.func_ptr_ret(
            payload->task.config.arg, 
            payload->arenas.persitent
        );
        *payload->task.meta.res_addr = output;
    } else {
        payload->task.config.func_ptr_no_ret(
            payload->task.config.arg
        );
    }
    atomic_store_explicit(&payload->task.meta.async_obj->is_done, true, memory_order_release);
    arena_giveback(&payload->arenas.scratch, payload, sizeof(thread_payload_t), 0);
    return 0;
}

void bgtask_manager_run_all_tasks(bgtask_manager_t *self)
{
    //TODO: maybe have this restrain to few threads only
    while(!queue_is_empty(&self->tasks)) {

        bgtask_t task = {0};
        queue_get_in_buffer(&self->tasks, &task, sizeof(task));

        arena_t scratch = arena_init(&self->arena, KB);
        thread_payload_t *payload = arena_reserve_raw(&scratch, sizeof(thread_payload_t));

        *payload = (thread_payload_t){
            .arenas = {
                .scratch = scratch,
                .persitent = task.config.arena
            },
            .task = task,
        };

        if (thrd_create(&payload->task.meta.async_obj->thrd.id, bgtask__internal_thread_wrapper, payload) != thrd_success) {
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
    async_object_t *obj = self;
#ifdef _WIN32
    thrd_exit(obj->thrd.id._Tid);
#else
    thrd_exit(obj->thrd.id);
#endif
}



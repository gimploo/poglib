#pragma once
#include <threads.h>
#include <stdatomic.h>
#include "./dbg.h"
#include "./ds.h"
#include "./common.h"

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
    void (*func_ptr)(task_args_t args, void **data_res);
    task_args_t arg;
} task_config_t;

typedef struct {
    task_config_t config;
    struct {
        async_object_t *async_obj;
        void **res_addr;
    } meta;
} bg_task_t;

typedef struct {

    queue_t tasks;

} bg_task_manager_t;

bg_task_manager_t bg_task_manager_init(void)
{
    return (bg_task_manager_t) {
        .tasks = queue_init(10, bg_task_t)
    };
}

void bg_task_manager_pass_task(
    bg_task_manager_t *self, 
    task_config_t config,
    async_object_t *async_obj
) {
    ASSERT(async_obj);

    async_obj->thrd.id = (thrd_t){0};

    const bg_task_t task = (bg_task_t){
        .config = config,
        .meta = {
            .async_obj = async_obj,
            .res_addr = &async_obj->resource
        }
    };
    queue_put(&self->tasks, task);
}

i32 __task_thread_wrapper(void *bg_task)
{
    bg_task_t *task = bg_task;
    task->config.func_ptr(
        task->config.arg, 
        task->meta.res_addr
    );
    atomic_store_explicit(&task->meta.async_obj->is_done, true, memory_order_release);
    mem_free(task, sizeof(bg_task_t));
    return 0;
}

void bg_task_manager_run_all_tasks(bg_task_manager_t *self)
{
    //TODO: maybe have this restrain to few threads only
    while(!queue_is_empty(&self->tasks)) {
        bg_task_t *task_buffer = mem_init(NULL, sizeof(bg_task_t));
        queue_get_in_buffer(&self->tasks, task_buffer, sizeof(bg_task_t));
        if (thrd_create(
            &task_buffer->meta.async_obj->thrd.id, 
            __task_thread_wrapper, 
            task_buffer) != thrd_success) {
            eprint("Failed to generate thread");
        }
    }
}

void bg_task_manager_destroy(bg_task_manager_t *self)
{
    queue_destroy(&self->tasks);
}



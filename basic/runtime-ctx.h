#pragma once
#include "./common.h"
#include "./arena.h"
#include "./arena_stack.h"

//NOTE: add members that would be used during runtime of an application

typedef struct runtimectx_t runtimectx_t;
struct runtimectx_t {
    arena_t *stringpool;
    stackarena_t *stackarena;
    buffer_t scrap_buffer;
};

extern runtimectx_t *global_runtimectx;

runtimectx_t *  runtimectx_init(void);
arena_t        *runtimectx_reserve_mem_from_stringpool(const u32 size);
void            runtimectx_destroy(void);


#ifndef IGNORE_RUNTIME_CTX_IMPLEMENTATION

runtimectx_t *global_runtimectx = NULL;

runtimectx_t * runtimectx_init(void)
{
    ASSERT(global_runtimectx == NULL);

    runtimectx_t * o = calloc(1, sizeof(runtimectx_t));

   *o = (runtimectx_t ){
        .stringpool = arena_init(NULL, 1 * MB),
        .scrap_buffer = (buffer_t) {
            .raw_data = calloc(4 * MB, 1),
            .size = 4 * MB
        },
        .stackarena = stackarena_init(1 * MB)
    };

    global_runtimectx = o;
    return global_runtimectx;
}

arena_t * runtimectx_reserve_mem_from_stringpool(const u32 size)
{
    ASSERT(global_runtimectx);
    return arena_init(global_runtimectx->stringpool, size);
}

void runtimectx_destroy(void)
{
    ASSERT(global_runtimectx);
    arena_destroy(global_runtimectx->stringpool);
    stackarena_destroy(global_runtimectx->stackarena);
    free(global_runtimectx->scrap_buffer.raw_data);
    free(global_runtimectx);
}

#endif

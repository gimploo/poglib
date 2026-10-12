#pragma once
#include "./common.h"
#include "./arena.h"
#include "./stack_arena.h"

//NOTE: add members that would be used during runtime of an application

typedef struct runtimectx_t runtimectx_t;
struct runtimectx_t {
    arena_t *stringpool;
    stackarena_t *stackarena;
    buffer_t scrap_buffer;
    arena_t *temporary;
};

extern runtimectx_t *global_runtimectx;

runtimectx_t *  runtimectx_init(void);
stackarena_t *  runtime_stackarena(void);
buffer_t        runtime_scratchbuffer(void);
arena_t *       runtimectx_reserve_mem_from_stringpool(const u32 size);
void            runtimectx_destroy(void);


#ifndef IGNORE_RUNTIME_CTX_IMPLEMENTATION

runtimectx_t *global_runtimectx = NULL;

runtimectx_t * runtimectx_init(void)
{
    ASSERT(global_runtimectx == NULL);

    runtimectx_t * o = calloc(1, sizeof(runtimectx_t));

   *o = (runtimectx_t ){
        .stringpool = arena_init(NULL),
        .scrap_buffer = (buffer_t) {
            .raw_data = calloc(4 * MB, 1),
            .size = 4 * MB
        },
        .stackarena = stackarena_init(0.5 * GB),
        .temporary = arena_init(NULL)
    };

    global_runtimectx = o;
    return global_runtimectx;
}

arena_t * runtimectx_reserve_mem_from_stringpool(const u32 size)
{
    ASSERT(global_runtimectx);
    return arena_init(global_runtimectx->stringpool);
}

void runtimectx_destroy(void)
{
    ASSERT(global_runtimectx);
    arena_destroy(global_runtimectx->stringpool);
    arena_destroy(global_runtimectx->temporary);
    stackarena_destroy(global_runtimectx->stackarena);
    free(global_runtimectx->scrap_buffer.raw_data);
    free(global_runtimectx);
}

stackarena_t * runtime_stackarena(void)
{
    ASSERT(global_runtimectx);
    return global_runtimectx->stackarena;
}

arena_t * runtime_scratcharena(void)
{
    ASSERT(global_runtimectx);
    arena_clear(global_runtimectx->temporary);
    return global_runtimectx->temporary;
}

buffer_t runtime_scratchbuffer(void)
{
    ASSERT(global_runtimectx);
    ASSERT(global_runtimectx->scrap_buffer.size);
    memset(global_runtimectx->scrap_buffer.raw_data, 0, global_runtimectx->scrap_buffer.size);
    return global_runtimectx->scrap_buffer;
}

#endif

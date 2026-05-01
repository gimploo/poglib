#pragma once
#include "./common.h"
#include "./dbg.h"
#include "./runtime-ctx.h"

void swap(void **x, void **y)
{
    void *t = *x;
    *x = *y;
    *y = t;
}

void swap_memory(void * const x, void * const y, const u64 size)
{
    ASSERT(global_runtimectx);
    buffer_t * const scrapbuffer = &global_runtimectx->scrap_buffer;

    ASSERT(size <= scrapbuffer->size);
    memset(scrapbuffer->raw_data, 0, sizeof(scrapbuffer->size));

    memcpy(scrapbuffer->raw_data, x, size);
    memcpy(x, y, size);
    memcpy(y, scrapbuffer->raw_data, size);
}

void * mem_init(const void *data, const u32 data_size)
{
    void *tmp = calloc(1, data_size);
    ASSERT(tmp);

    if (data) {
        memcpy(tmp, data, data_size);
    }
    return tmp;
}

void mem_free(void *data, const u32 data_size)
{
    ASSERT(data);
    memset(data, 0, data_size);
    free(data);
}

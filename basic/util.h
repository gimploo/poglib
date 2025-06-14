#pragma once
#include "./common.h"
#include "./dbg.h"

void swap(void **x, void **y)
{
    void *t = *x;
    *x = *y;
    *y = t;
}

static u8 __tmp_scrap_memory[4 * MB] = {0};

void swap_memory(void * const x, void * const y, const u64 size)
{
    ASSERT(size <= sizeof(__tmp_scrap_memory));

    memcpy(__tmp_scrap_memory, x, size);
    memcpy(x, y, size);
    memcpy(y, __tmp_scrap_memory, size);
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
    memset(data, 0, data_size);
    free(data);
}

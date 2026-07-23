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

u32 get_index_from_bitflag(const u32 bitflag) {
    ASSERT(bitflag != 0);

#if defined(_MSC_VER)
    // MSVC (Windows) intrinsic
    unsigned long index;
    _BitScanForward(&index, bitflag);
    return index;
#elif defined(__GNUC__) || defined(__clang__)
    // GCC / Clang (Linux/MinGW) intrinsic
    return __builtin_ctz(bitflag);
#else
    // Fallback: De Bruijn sequence or simple loop if intrinsics are unavailable
    uint32_t index = 0;
    while ((bitflag & 1) == 0) {
        bitflag >>= 1;
        index++;
    }
    return index;
#endif
}


buffer_t buffer_init(void *data, const u32 size)
{
    return (buffer_t) {
        .raw_data = mem_init(data, size),
        .size = size,
        .is_on_heap = true
    };
}

void buffer_destroy(buffer_t *const self)
{
    ASSERT(self->is_on_heap);
    mem_free(self->raw_data, self->size);
}


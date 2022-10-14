#pragma once
#include "ds/list.h"

//code from gunslinger

typedef struct bytebuffer_t
{
    uint8_t     *data;      // Buffer that actually holds all relevant byte data
    uint32_t    size;      // Current size of the stored buffer data
    uint32_t    position;  // Current read/write position in the buffer
    uint32_t    capacity;  // Current max capacity for the buffer
                        
} bytebuffer_t;

bytebuffer_t bytebuffer_init(void)
{
    return (bytebuffer_t ) {
        .data     = (uint8_t*)calloc(1, 1024),
        .size     = 0,
        .position = 0,
        .capacity = 1024,
    };
}

void bytebuffer_destroy(bytebuffer_t *self)
{
    free(self->data);
    self->data = NULL;
}

void bytebuffer_clear(bytebuffer_t *buffer)
{
    buffer->size = 0;
    buffer->position = 0;   
}

void __bytebuffer_resize(bytebuffer_t *buffer, size_t sz)
{
    uint8_t* data = (uint8_t*)realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;    
    buffer->capacity = (uint32_t)sz;
}

// Generic "write" function for a byte buffer
#define bytebuffer_write(__BB, __T, __VAL)\
do {\
    bytebuffer_t* __BUFFER = __BB;\
    u64 __SZ = sizeof(__T);\
    u64 __TWS = __BUFFER->position + __SZ;\
    if (__TWS >= (u64)__BUFFER->capacity)\
    {\
        u64 __CAP = __BUFFER->capacity * 2;\
        while(__CAP < __TWS)\
        {\
            __CAP *= 2;\
        }\
        __bytebuffer_resize(__BUFFER, __CAP);\
    }\
    *(__T*)(__BUFFER->data + __BUFFER->position) = __VAL;\
    __BUFFER->position += (uint32_t)__SZ;\
    __BUFFER->size += (uint32_t)__SZ;\
} while (0)

// Generic "read" function
#define bytebuffer_read(__BUFFER, __T, __VAL_P)\
do {\
    __T* __V = (__T*)(__VAL_P);\
    bytebuffer_t* __BB = (__BUFFER);\
    *(__V) = *(__T*)(__BB->data + __BB->position);\
    __BB->position += sizeof(__T);\
} while (0)

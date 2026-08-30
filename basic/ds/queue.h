#pragma once
#include "../dbg.h"
#include "../common.h"
#include <poglib/basic/arena.h>

/*=============================================================================
                            - QUEUE DATA STRUCTURE -
=============================================================================*/

typedef struct {

    u64     len;
    u8      *__data;
    u64     __start;
    u64     __end;
    u64     __capacity;
    u64     __elem_size;
    bool    __are_values_pointers;
    struct {
        arena_t *arena;
    } internal;
} queue_t ;


#define             queue_init(CAPACITY, TYPE, PARENA)                          __impl_queue_init((CAPACITY), sizeof(TYPE), #TYPE, (PARENA))

void                queue_put(queue_t *const self, const void *const elemaddr, const u64 elem_size);
#define             queue_get(PQUEUE)                                           __impl_queue_get((PQUEUE)) 
void                queue_get_in_buffer(queue_t *queue, buffer_t buffer);
#define             queue_is_empty(PQUEUE)                                      ((PQUEUE)->__start == (PQUEUE)->__end)
#define             queue_is_full(PQUEUE)                                       ((PQUEUE)->len == (PQUEUE)->__capacity) ? true : false

#define             queue_iterator(PQUEUE, ITER)                                __impl_queue_iterator((PQUEUE), (ITER))

void                queue_clear(queue_t *queue);
void                queue_print(queue_t *queue, void (*print)(void *));
void                queue_dump(queue_t *queue);

void                *queue_peek(const queue_t * const );

void                queue_destroy(queue_t *queue);


/*-----------------------------------------------------------------------------
                                IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_QUEUE_IMPLEMENTATION

void queue_destroy(queue_t *queue)
{
    assert(queue);
    if (queue->internal.arena) {
        arena_giveback(queue->internal.arena, queue->__data, queue->__elem_size * queue->__capacity);
    } else {
        free(queue->__data);
    }
    queue->__data = NULL;
    queue->__start = queue->__end = queue->len = 0; 

}

#define __impl_queue_iterator(PQUEUE, ITER)\
    if ((PQUEUE)->len != 0)\
        for (void **index = 0, *(ITER) = (void *)__queue_get_value_at_index((PQUEUE), (u64)index);\
                (u64)(index) < (PQUEUE)->len;\
                index = (void **)((u64)index + 1),\
                (ITER) = (void *)__queue_get_value_at_index(PQUEUE, \
                    ((u64)index < (PQUEUE)->len) ? (u64)index : (u64)index - 1))

void queue_dump(queue_t *queue)
{
    if (queue == NULL) eprint("queue_dump: queue argument is null");

    printf(
        "\n"
        " queue->__start       = %li,\n" 
        " queue->__end         = %li,\n"
        " queue->__capacity    = %li,\n" 
        " queue->__elem_size   = %li,\n" 
        " queue->len         = %li,\n"
        " queue->__are_values_pointers = %i\n",

        queue->__start,
        queue->__end,
        queue->__capacity,
        queue->__elem_size,
        queue->len,
        queue->__are_values_pointers
    );

    printf(" queue->__data       = [");
    for (u64 i = 0; i < queue->__capacity; i++)
        fprintf(stdout, "%p ", queue->__data + i * queue->__elem_size);
    printf("]\n\n");
}



queue_t __impl_queue_init(u64 capacity, u64 elem_size, const char *elem_type, arena_t * const arena)
{
    ASSERT(capacity > 0);
    ASSERT(elem_type);
    ASSERT(elem_size > 0);
    ASSERT(arena);

    //TODO: inline values for element size <= 8 i.e sizeof(void *)

    return (queue_t) {
        .len = 0 ,
        .__data = arena_reserve(arena, elem_size * capacity),
        .__start = 0,
        .__end = 0,
        .__capacity = capacity,
        .__elem_size = elem_size,
        .__are_values_pointers = sizeof(void *) == elem_size,
        .internal = {
            .arena = arena,
        }
    };
}

void queue_clear(queue_t *queue)
{
    assert(queue);

    queue->__start = queue->len = queue->__end = 0;
    memset(queue->__data, 0, queue->__capacity * queue->__elem_size);

}

void queue_put(queue_t *const self, const void *elemaddr, const u64 elemsize)
{
    if (self == NULL)                   eprint("queue_put: queue argument is null");
    if (elemaddr == NULL)               eprint("queue_put: elem argument is null");
    if (elemsize != self->__elem_size)  eprint("expected elem_size %llu but got %llu", self->__elem_size, elemsize);

    if (queue_is_full(self)) eprint("overflow (queue size `%lli`)", self->__capacity);

    void *dest = (self->__data + self->__end * self->__elem_size);
    if (self->__are_values_pointers)    memcpy(dest, &elemaddr, sizeof(void *));
    else                                memcpy(dest, elemaddr, self->__elem_size);

    self->len++;

    u64 oldpos = self->__end;
    self->__end = (self->__end + 1) % self->__capacity;
    if (self->__end == self->__start) self->__end = oldpos;
}

void * __queue_get_value_at_index(const queue_t *queue, const u64 index)
{
    if (queue == NULL)          eprint("queue_get: queue argument is null");
    if (queue_is_empty(queue))  eprint("underflow");
    if (index >= queue->len)     eprint("index exceeds the length of the queue");

    void *elem_pos = NULL;
    if (queue->__are_values_pointers)
        elem_pos  = *(void **)(queue->__data + index * queue->__elem_size);
    else
        elem_pos  = (queue->__data + index * queue->__elem_size);

    return elem_pos;
}

void * __impl_queue_get(queue_t *queue)
{
    if (queue == NULL)              eprint("queue_get: queue argument is null");
    if (queue->__elem_size > 8)     eprint("element size exceeds 8 bytes, use queue_get_in_buffer() instead");
    if (queue_is_empty(queue))      eprint("underflow");

    void *elem_pos = NULL;
    if (queue->__are_values_pointers)
        elem_pos  = *(void **)(queue->__data + queue->__start * queue->__elem_size);
    else
        elem_pos  = (queue->__data + queue->__start * queue->__elem_size);

    queue->__start    = (queue->__start + 1) % queue->__capacity;
    queue->len--;

    return elem_pos;
}

void queue_get_in_buffer(queue_t *queue, buffer_t buffer)
{
    if (queue == NULL)                      eprint("queue_get: queue argument is null");
    if (queue_is_empty(queue))              eprint("underflow");
    if (buffer.size < queue->__elem_size)   eprint("buffer is too smol, expected %llu but given %u", queue->__elem_size, buffer.size);

    void *elem_pos = NULL;
    if (queue->__are_values_pointers)
        elem_pos  = *(void **)(queue->__data + queue->__start * queue->__elem_size);
    else
        elem_pos  = (queue->__data + queue->__start * queue->__elem_size);

    queue->__start    = (queue->__start + 1) % queue->__capacity;
    queue->len--;

    memcpy(buffer.raw_data, elem_pos, queue->__elem_size);
}


void queue_print(queue_t *queue, void (*print_elem)(void *))
{
    if (queue == NULL) eprint("queue_print: queue argument is null");

    if(queue_is_empty(queue)) {
        printf("queue is empty\n");
        return;
    } 

    void *elem_pos = NULL;
    for (u64 i = queue->__start, j = 0; j < queue->len; i = (i + 1) % queue->len, j++)
    {
        if (queue->__are_values_pointers)
            elem_pos  = *(void **)(queue->__data + i * queue->__elem_size);
        else
            elem_pos  = (queue->__data + i * queue->__elem_size);

        print_elem(elem_pos);
    }
    printf("\n");
}


void * queue_peek(const queue_t * const self) {
    ASSERT(self->len);
    return self->__data + self->__end * self->__elem_size;
}


#endif 

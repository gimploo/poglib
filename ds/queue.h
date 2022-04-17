#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../basic.h"




typedef struct queue_t queue_t ;



#define             queue_init(CAPACITY, TYPE)          __impl_queue_init(CAPACITY, sizeof(TYPE), #TYPE)

#define             queue_put(PQUEUE, ELEM)             __impl_queue_put((PQUEUE), &(ELEM), sizeof(ELEM))

#define             queue_get(PQUEUE)                   __impl_queue_get((PQUEUE)) 
#define             queue_get_in_buffer(PQUEUE, BUFFER) __impl_queue_get_in_buffer((PQUEUE), &BUFFER, sizeof(BUFFER))

#define             queue_is_empty(PQUEUE)              ((PQUEUE)->__start == (PQUEUE)->__end)
#define             queue_is_full(PQUEUE)               ((PQUEUE)->len == (PQUEUE)->__capacity) ? true : false


void                queue_clear(queue_t *queue);

void                queue_destroy(queue_t *queue);

void                queue_print(queue_t *queue, void (*print)(void *));
void                queue_dump(queue_t *queue);








#ifndef IGNORE_QUEUE_IMPLEMENTATION

struct queue_t {

    u64     len;
    u8      *__array;
    u64     __start;
    u64     __end;
    u64     __capacity;
    u64     __elem_size;
    bool    are_values_pointers;   // This variable checks if the list is a list of pointers 
};

void queue_destroy(queue_t *queue)
{
    assert(queue);

    free(queue->__array);
    queue->__array = NULL;
    queue->__start = queue->__end = queue->len = 0; 

}

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
        " queue->are_values_pointers = %i\n",

        queue->__start,
        queue->__end,
        queue->__capacity,
        queue->__elem_size,
        queue->len,
        queue->are_values_pointers
    );

    printf(" queue->__array       = [");
    for (u64 i = 0; i < queue->__capacity; i++)
        fprintf(stdout, "%p ", queue->__array + i * queue->__elem_size);
    printf("]\n\n");
}



queue_t __impl_queue_init(u64 __capacity, u64 __elem_size, const char *type_in_strings)
{
    if (__capacity == 0) eprint("queue_init: __capacity not greater than zero");
    assert(__elem_size != 0);

    bool flag = false;
    if (type_in_strings[strlen(type_in_strings) - 1] == '*') flag = true;

    return (queue_t) {
        .__array = (u8 *)calloc(__capacity, __elem_size),
        .__start = 0,
        .__end = 0,
        .__capacity = __capacity,
        .__elem_size = __elem_size,
        .len = 0 ,
        .are_values_pointers = flag
    };
}

void queue_clear(queue_t *queue)
{
    assert(queue);

    queue->__start = queue->len = queue->__end = 0;
    memset(queue->__array, 0, queue->__capacity * queue->__elem_size);

}

void __impl_queue_put(queue_t *queue, const void *elemaddr, u64 __elem_size)
{
    if (queue == NULL)                  eprint("queue_put: queue argument is null");
    if (elemaddr == NULL)               eprint("queue_put: elem argument is null");
    if (__elem_size != queue->__elem_size)  eprint("expected __elem_size %li but got %li", queue->__elem_size, __elem_size);

    if (queue_is_full(queue)) eprint("overflow");

    // pass by value
    memcpy(queue->__array + (queue->__end * queue->__elem_size), 
            elemaddr, queue->__elem_size);
    queue->len++;

    u64 oldpos = queue->__end;
    queue->__end = (queue->__end + 1) % queue->__capacity;
    if (queue->__end == queue->__start) queue->__end = oldpos;
}


void * __impl_queue_get(queue_t *queue)
{
    if (queue == NULL)          eprint("queue_get: queue argument is null");
    if (queue->__elem_size > 8)   eprint("element size exceeds 8 bytes, use queue_get_in_buffer() instead");
    if (queue_is_empty(queue))  eprint("underflow");

    void *elem_pos = NULL;
    if (queue->are_values_pointers)
        elem_pos  = *(void **)(queue->__array + queue->__start * queue->__elem_size);
    else
        elem_pos  = (queue->__array + queue->__start * queue->__elem_size);

    queue->__start    = (queue->__start + 1) % queue->__capacity;
    queue->len--;

    return elem_pos;
}

void __impl_queue_get_in_buffer(queue_t *queue, void *buffer, u64 buffer_size)
{
    if (queue == NULL)                   eprint("queue_get: queue argument is null");
    if (queue_is_empty(queue))           eprint("underflow");
    //if (queue->__elem_size <= 8)           eprint("Use normal queue_get() instead");
    if (buffer_size < queue->__elem_size)  eprint("buffer is too smol, expected %lu but given %lu", queue->__elem_size, buffer_size);
    if (buffer_size == 8)                eprint("passed in buffer is a pointer");

    void *elem_pos = NULL;
    if (queue->are_values_pointers)
        elem_pos  = *(void **)(queue->__array + queue->__start * queue->__elem_size);
    else
        elem_pos  = (queue->__array + queue->__start * queue->__elem_size);

    queue->__start    = (queue->__start + 1) % queue->__capacity;
    queue->len--;

    memcpy(buffer, elem_pos, queue->__elem_size);
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
        if (queue->are_values_pointers)
            elem_pos  = *(void **)(queue->__array + i * queue->__elem_size);
        else
            elem_pos  = (queue->__array + i * queue->__elem_size);

        print_elem(elem_pos);
    }
    printf("\n");
}



#endif 

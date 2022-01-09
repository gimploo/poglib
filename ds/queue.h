#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../basic.h"




typedef struct queue_t queue_t ;



#define             queue_init(CAPACITY, TYPE)          __impl_queue_init(CAPACITY, sizeof(TYPE))

#define             queue_put(PQUEUE, ELEM)             __impl_queue_put((PQUEUE), &(ELEM), sizeof(ELEM))

#define             queue_get(PQUEUE)                   __impl_queue_get((PQUEUE)) 
#define             queue_get_in_buffer(PQUEUE, BUFFER)  __impl_queue_get_in_buffer((PQUEUE), &BUFFER, sizeof(BUFFER))

#define             queue_is_empty(PQUEUE)              ((PQUEUE)->start == (PQUEUE)->end) 
#define             queue_is_full(PQUEUE)               ((PQUEUE)->len == (PQUEUE)->capacity) ? true : false

#define             queue_peek_element_at_index(PQUEUE, INDEX)            ((PQUEUE)->array + (INDEX) * (PQUEUE)->elem_size)

void                queue_clear(queue_t *queue);

void                queue_destroy(queue_t *queue);

void                queue_print(queue_t *queue, void (*print)(void *));
void                queue_dump(queue_t *queue);








#ifndef IGNORE_QUEUE_IMPLEMENTATION

struct queue_t {

    u8 *array;
    u64 start;
    u64 end;
    u64 capacity;
    u64 elem_size;
    u64 len;

};

void queue_destroy(queue_t *queue)
{
    assert(queue);

    free(queue->array);
    queue->array = NULL;
    queue->start = queue->end = queue->len = 0; 

}

void queue_dump(queue_t *queue)
{
    if (queue == NULL) eprint("queue_dump: queue argument is null");

    fprintf(stderr, "[ERROR] DUMPING QUEUE\n");
    for (u64 i = 0; i < queue->capacity; i++)
        fprintf(stdout, "%p ", queue->array + i * queue->elem_size);
    printf("\n");
}



queue_t __impl_queue_init(u64 capacity, u64 elem_size)
{
    if (capacity == 0) eprint("queue_init: capacity not greater than zero");
    assert(elem_size != 0);

    return (queue_t) {
        .array = (u8 *)calloc(capacity, elem_size),
        .start = 0,
        .end = 0,
        .capacity = capacity,
        .elem_size = elem_size,
        .len = 0 ,
    };
}

void queue_clear(queue_t *queue)
{
    assert(queue);

    queue->start = queue->len = queue->end = 0;

}

void __impl_queue_put(queue_t *queue, void *elemaddr, u64 elem_size)
{
    if (queue == NULL) eprint("queue_put: queue argument is null");
    if (elemaddr == NULL) eprint("queue_put: elem argument is null");
    if (elem_size != queue->elem_size) eprint("expected elem_size %li but got %li", queue->elem_size, elem_size);

    if (queue_is_full(queue)) eprint("overflow");

    // pass by value
    memcpy(queue->array + (queue->end * queue->elem_size), 
            elemaddr, queue->elem_size);
    queue->len++;

    u64 oldpos = queue->end;
    queue->end = (queue->end + 1) % queue->capacity;
    if (queue->end == queue->start) queue->end = oldpos;
}

void * __impl_queue_get(queue_t *queue)
{
    if (queue == NULL) eprint("queue_get: queue argument is null");
    if (queue->elem_size > 8) eprint("element size exceeds 8 bytes, use queue_get_in_buffer() instead");

    if (queue_is_empty(queue)) eprint("underflow");

    void *elem_pos = queue->array + queue->start * queue->elem_size;
    queue->start = (queue->start + 1) % queue->capacity;
    queue->len--;

    return elem_pos;
}

void __impl_queue_get_in_buffer(queue_t *queue, void *buffer, u64 buffer_size)
{
    if (queue == NULL)                  eprint("queue_get: queue argument is null");
    if (queue_is_empty(queue))          eprint("underflow");
    if(queue->elem_size <= 8)           eprint("Use normal queue_get() instead");
    if(buffer_size < queue->elem_size)  eprint("buffer is too smol, expected %lu but given %lu", queue->elem_size, buffer_size);
    if(buffer_size == 8)                eprint("passed in buffer is a pointer");

    void *elem_pos = queue->array + queue->start * queue->elem_size;
    queue->start = (queue->start + 1) % queue->capacity;
    queue->len--;

    memcpy(buffer, elem_pos, queue->elem_size);
}


void queue_print(queue_t *queue, void (*print_elem)(void *))
{
    if (queue == NULL) eprint("queue_print: queue argument is null");

    if(queue_is_empty(queue)) {
        printf("queue is empty\n");
        return;
    } 

    for (u64 i = queue->start, j = 0; j < queue->len; i = (i + 1) % queue->capacity, j++)
    {
        print_elem(queue->array + i * queue->elem_size);
    }
    printf("\n");
}



#endif 

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../basic.h"




typedef struct queue_t queue_t ;



#define             queue_init(CAPACITY, TYPE)          __impl_queue_init(CAPACITY, sizeof(TYPE))

#define             queue_put(PQUEUE, ELEM)             __impl_queue_put((PQUEUE), &(ELEM), sizeof(ELEM))

//NOTE: if the value exceeds 8 bytes in size, the queue returns a buffer holding the value 
//      so to be copied to someother place else it just returns the value itself if its 
//      less than 8 bytes
#define             queue_get(PQUEUE)                   __impl_queue_get((PQUEUE)) 

#define             queue_is_empty(pqueue)              ((pqueue)->len == 0) ? true : false
#define             queue_is_full(pqueue)               ((pqueue)->len == (pqueue)->capacity) ? true : false

void                queue_destroy(queue_t *queue);

void                queue_print(queue_t *queue, void (*print)(void *));
void                queue_dump(queue_t *queue);








#ifndef IGNORE_QUEUE_IMPLEMENTATION

struct queue_t {

    u8 *array;
    i64 start;
    i64 end;
    u64 capacity;
    u64 elem_size;
    u64 len;

    // Buffer that stores the get values from queue_get calls instead of having to pass 
    // a buffer as argument to store the output
    void *buffer; 

};

void queue_destroy(queue_t *queue)
{
    assert(queue);

    free(queue->array);
    queue->array = NULL;
    queue->start = queue->end = queue->len = 0; 

    free(queue->buffer);
    queue->buffer = NULL;
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


    return (queue_t) {
        .array = (u8 *)calloc(capacity, elem_size),
        .start = -1,
        .end = -1,
        .capacity = capacity,
        .elem_size = elem_size,
        .len = 0 ,
        .buffer = (void *)calloc(1, elem_size) 
    };
}

void __impl_queue_put(queue_t *queue, void *elemaddr, u64 elem_size)
{
    if (queue == NULL) eprint("queue_put: queue argument is null");
    if (elemaddr == NULL) eprint("queue_put: elem argument is null");
    if (elem_size != queue->elem_size) eprint("expected elem_size %li but got %li", queue->elem_size, elem_size);

    if (queue_is_full(queue)) {

        queue_dump(queue);
        eprint("overflow");

    } else if (queue_is_empty(queue)) {

        queue->start = 0;
        queue->end = 0;

    } else {

        queue->end = (++queue->end) % queue->capacity;

    }

    // pass by value
    memcpy(queue->array + (queue->end * queue->elem_size), 
            elemaddr, queue->elem_size);

    queue->len++;
}

void * __impl_queue_get(queue_t *queue)
{
    if (queue == NULL) eprint("queue_get: queue argument is null");

    if (queue_is_empty(queue)) {

        queue_dump(queue);
        eprint("underflow");

    } 

    void *elem_pos = queue->array + queue->start * queue->elem_size;
    queue->start = ++queue->start % queue->capacity;
    queue->len--;

    // if the value is a pointer or have size less than a pointer, returns the value
    if (queue->elem_size <= 8) return elem_pos;

    // else it returns in a buffer
    memcpy(queue->buffer, elem_pos, queue->elem_size);
    return queue->buffer;

}

void queue_print(queue_t *queue, void (*print_elem)(void *))
{
    if (queue == NULL) eprint("queue_print: queue argument is null");

    if(queue_is_empty(queue)) {
        printf("queue is empty\n");
        return;
    } 

    for (u64 i = queue->start, j = 0; j < queue->len; i = (++i) % queue->capacity, j++)
    {
        print_elem(queue->array + i * queue->elem_size);
    }
    printf("\n");
}



#endif 

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

#define             queue_is_empty(PQUEUE)              ((PQUEUE)->start == (PQUEUE)->end) 
#define             queue_is_full(PQUEUE)               ((PQUEUE)->len == (PQUEUE)->capacity) ? true : false

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


    return (queue_t) {
        .array = (u8 *)calloc(capacity, elem_size),
        .start = 0,
        .end = 0,
        .capacity = capacity,
        .elem_size = elem_size,
        .len = 0 ,
    };
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
    queue->end = (++queue->end) % queue->capacity;
    if (queue->end == queue->start) queue->end = oldpos;
}

void * __impl_queue_get(queue_t *queue)
{
    if (queue == NULL) eprint("queue_get: queue argument is null");

    if (queue_is_empty(queue)) eprint("underflow");

    void *elem_pos = queue->array + queue->start * queue->elem_size;
    queue->start = ++queue->start % queue->capacity;
    queue->len--;

    return elem_pos;
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

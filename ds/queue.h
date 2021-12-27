#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../basic.h"




typedef struct queue_t queue_t ;

#define     queue_init(PARR, SIZE, CAPACITY) __impl_queue_init((void **)PARR, SIZE, CAPACITY)
#define     queue_put(PQUEUE, ELEM) __impl_queue_put((PQUEUE), (ELEM), &(ELEM), sizeof(ELEM))

//NOTE: if the value exceeds 8 bytes in size, the queue returns a buffer holding the value 
//      so to be copied to someother place else it just returns the value itself if its 
//      less than 8 bytes
#define     queue_get(PQUEUE) __impl_queue_get((PQUEUE)) 

#define     queue_is_empty(pqueue)  ((pqueue)->len == 0) ? true : false
#define     queue_is_full(pqueue)   ((pqueue)->len == (pqueue)->capacity) ? true : false

void        queue_destroy(queue_t *queue);

void        queue_print(queue_t *queue, void (*print)(void *));
void        queue_dump(queue_t *queue);








#ifndef IGNORE_QUEUE_IMPLEMENTATION

struct queue_t {

    void **array;
    i64 start;
    i64 end;
    const u64 capacity;
    const u64 elem_size;
    u64 len;

    // Buffer that stores the get values from queue_get calls instead of having to pass 
    // a buffer to have it store the output
    void *buffer; 

};

void queue_destroy(queue_t *queue)
{
    assert(queue);

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
        fprintf(stdout, "%p ", queue->array[i]);
    printf("\n");
}



queue_t __impl_queue_init(void **array, u64 size, u64 capacity)
{
    if (array == NULL) eprint("queue_init: array argument is null");
    if (capacity == 0) eprint("queue_init: capacity not greater than zero");

    memset(array, 0, size);

    return (queue_t) {
        .array = array,
        .start = -1,
        .end = -1,
        .capacity = capacity,
        .elem_size = size / capacity,
        .len = 0 ,
        .buffer = (void *)calloc(1, size/capacity) 
    };
}

void __impl_queue_put(queue_t *queue, void *elem, void *elemaddr, u64 elem_size)
{
    if (queue == NULL) eprint("queue_put: queue argument is null");
    if (elem == NULL && elemaddr == NULL) eprint("queue_put: elem argument is null");
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


    if (elem_size <= 8) {

        // pass by reference
        queue->array[queue->end] = elem;

    } else {

        // pass by value
        memcpy(queue->array + (queue->end * queue->elem_size), 
                elemaddr, queue->elem_size);
    }

    queue->len++;
}

void * __impl_queue_get(queue_t *queue)
{
    if (queue == NULL) eprint("queue_get: queue argument is null");

    if (queue_is_empty(queue)) {

        queue_dump(queue);
        eprint("underflow");

    } 

    if (queue->elem_size <= 8)
    {
        void *elem = queue->array[queue->start];
        queue->start = ++queue->start % queue->capacity;
        queue->len--;
        return elem;
    }

    void *elem_pos = queue->array + queue->start * queue->elem_size;
    queue->start = ++queue->start % queue->capacity;
    queue->len--;

    memcpy(queue->buffer, elem_pos, queue->elem_size);

    return queue->buffer;

}

void queue_print(queue_t *queue, void (*print_elem)(void *))
{
    if (queue == NULL) eprint("queue_print: queue argument is null");

    if(queue_is_empty(queue)) {
        printf("queue is empty\n");
    } else {
        for (u64 i = queue->start, j = 0; j < queue->len; i = (++i) % queue->capacity, j++)
        {
            if (queue->elem_size <= 8)
                print_elem(queue->array[i]);
            else
                print_elem(queue->array + i * queue->elem_size);
        }
        printf("\n");
    }

}



#endif 

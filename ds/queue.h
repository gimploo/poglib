#ifndef __MY_QUEUE_H__
#define __MY_QUEUE_H__

#include <stdio.h>
#include <stdlib.h>

#include "../basic.h"

/*=============================================================
 // Generic Queue datastructure
=============================================================*/

typedef struct queue_t {

    void **array;
    i64 start;
    i64 end;
    u64 capacity;

} queue_t;

/*--------------------------------------------------------------
 // Declarations
--------------------------------------------------------------*/
static inline queue_t queue_init(void **array, size_t capacity);
static inline void queue_put(queue_t *queue, void *elem);
static inline void * queue_get(queue_t *queue);

/*--------------------------------------------------------------
 // Implementation
--------------------------------------------------------------*/

static inline void queue_dump(queue_t *queue)
{
    if (queue == NULL) eprint("queue_dump: queue argument is null");

    fprintf(stderr, "[ERROR] DUMPING QUEUE\n");
    for (u64 i = 0; i < queue->capacity; i++)
        fprintf(stdout, "%p ", queue->array[i]);
    printf("\n");
}

#define queue_is_empty(pqueue)  (((pqueue)->start == -1 && (pqueue)->end == -1) ? true : false)
#define queue_is_full(pqueue)   ((pqueue)->end == ((pqueue)->capacity - 1) ? true : false)


static inline queue_t queue_init(void **array, size_t capacity)
{
    if (array == NULL) eprint("queue_init: array argument is null");
    if (capacity == 0) eprint("queue_init: capacity not greater than zero");

    for (u64 i = 0; i < capacity; i++) array[i] = NULL;

    return (queue_t) {
        .array = array,
        .start = -1,
        .end = -1,
        .capacity = capacity
    };
}

static inline void queue_put(queue_t *queue, void *elem)
{
    if (queue == NULL) eprint("queue_put: queue argument is null");
    if (elem == NULL) eprint("queue_put: elem argument is null");

    if (queue_is_full(queue)) {

        queue_dump(queue);
        eprint("overflow");

    } else if (queue_is_empty(queue)) {

        queue->end = queue->start = 0;
        queue->array[++queue->end] = elem;
        return ;
    }
    
    queue->end = (queue->end + 1) % queue->capacity;
    queue->array[queue->end] = elem;
}

static inline void * queue_get(queue_t *queue)
{
    if (queue == NULL) eprint("queue_get: queue argument is null");

    if (queue_is_empty(queue)) {

        queue_dump(queue);
        eprint("underflow");

    } else if (queue->start == queue->end) {

        i64 pos = queue->start;
        queue->start = queue->end = -1;
        void *elem = queue->array[pos];
        queue->array[pos] = NULL;
        return elem;
    }


    void *elem = queue->array[queue->start];
    queue->array[queue->start] = NULL;
    queue->start = (queue->start + 1) % queue->capacity;

    return elem;
}

static inline void queue_print(queue_t *queue, void (*print_elem)(void *))
{
    if (queue == NULL) eprint("queue_print: queue argument is null");
    if(queue_is_empty(queue)) eprint("queue_print: queue is empty");

    for (int i = queue->start; i <= queue->end; i = (i+1) % queue->capacity)
        print_elem(queue->array[i]);

    printf("\n");
}


#endif // __MY_QUEUE_H__

#pragma once
#include "../dbg.h"
#include "../common.h"

/*=============================================================================
                            - QUEUE DATA STRUCTURE -
=============================================================================*/

typedef struct queue_t {

    u64     len;
    u8      *__data;
    u64     __start;
    u64     __end;
    u64     __capacity;
    u64     __elem_size;
    char    __elem_type[MAX_TYPE_CHARACTER_LENGTH];
    bool    __are_values_pointers;   // This variable checks if the list is a list of pointers 
                                   
} queue_t ;


#define             queue_init(CAPACITY, TYPE)                                  __impl_queue_init(CAPACITY, sizeof(TYPE), #TYPE)

#define             queue_put(PQUEUE, ELEM)                                     __impl_queue_put((PQUEUE), &(ELEM), sizeof(ELEM))
#define             queue_get(PQUEUE)                                           __impl_queue_get((PQUEUE)) 
#define             queue_get_in_buffer(PQUEUE, BUFFER)                         __impl_queue_get_in_buffer((PQUEUE), &BUFFER, sizeof(BUFFER))

#define             queue_is_empty(PQUEUE)                                      ((PQUEUE)->__start == (PQUEUE)->__end)
#define             queue_is_full(PQUEUE)                                       ((PQUEUE)->len == (PQUEUE)->__capacity) ? true : false

#define             queue_iterator(PQUEUE, ITER)                                __impl_queue_iterator((PQUEUE), (ITER))

void                queue_clear(queue_t *queue);
void                queue_print(queue_t *queue, void (*print)(void *));
void                queue_dump(queue_t *queue);

void                queue_destroy(queue_t *queue);


/*-----------------------------------------------------------------------------
                                IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_QUEUE_IMPLEMENTATION

void queue_destroy(queue_t *queue)
{
    assert(queue);

    free(queue->__data);
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



queue_t __impl_queue_init(u64 capacity, u64 elem_size, const char *elem_type)
{
    assert(capacity > 0);
    assert(elem_type);
    assert(elem_size > 0);

    bool flag = false;
    u32 len = strlen(elem_type);
    if (elem_type[len] > MAX_TYPE_CHARACTER_LENGTH) eprint("variable type is too big, exceeded the %i limit threshold\n", MAX_TYPE_CHARACTER_LENGTH);
    if (elem_type[len - 1] == '*') flag = true;

    queue_t o = {
        .len = 0 ,
        .__data = (u8 *)calloc(capacity, elem_size),
        .__start = 0,
        .__end = 0,
        .__capacity = capacity,
        .__elem_size = elem_size,
        .__elem_type = {0},
        .__are_values_pointers = flag
    };

    if (!flag)  memcpy(o.__elem_type, elem_type, MAX_TYPE_CHARACTER_LENGTH);
    else        memcpy(o.__elem_type, elem_type, len - 1);

    return o;
}

void queue_clear(queue_t *queue)
{
    assert(queue);

    queue->__start = queue->len = queue->__end = 0;
    memset(queue->__data, 0, queue->__capacity * queue->__elem_size);

}

void __impl_queue_put(queue_t *queue, const void *elemaddr, u64 __elem_size)
{
    if (queue == NULL)                  eprint("queue_put: queue argument is null");
    if (elemaddr == NULL)               eprint("queue_put: elem argument is null");
    if (__elem_size != queue->__elem_size)  eprint("expected __elem_size %li but got %li", queue->__elem_size, __elem_size);

    if (queue_is_full(queue)) eprint("overflow (queue size `%li`)", queue->__capacity);

    // pass by value
    memcpy(queue->__data + (queue->__end * queue->__elem_size), 
            elemaddr, queue->__elem_size);
    queue->len++;

    u64 oldpos = queue->__end;
    queue->__end = (queue->__end + 1) % queue->__capacity;
    if (queue->__end == queue->__start) queue->__end = oldpos;
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
    if (queue == NULL)          eprint("queue_get: queue argument is null");
    if (queue->__elem_size > 8)   eprint("element size exceeds 8 bytes, use queue_get_in_buffer() instead");
    if (queue_is_empty(queue))  eprint("underflow");

    void *elem_pos = NULL;
    if (queue->__are_values_pointers)
        elem_pos  = *(void **)(queue->__data + queue->__start * queue->__elem_size);
    else
        elem_pos  = (queue->__data + queue->__start * queue->__elem_size);

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
    if (queue->__are_values_pointers)
        elem_pos  = *(void **)(queue->__data + queue->__start * queue->__elem_size);
    else
        elem_pos  = (queue->__data + queue->__start * queue->__elem_size);

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
        if (queue->__are_values_pointers)
            elem_pos  = *(void **)(queue->__data + i * queue->__elem_size);
        else
            elem_pos  = (queue->__data + i * queue->__elem_size);

        print_elem(elem_pos);
    }
    printf("\n");
}



#endif 

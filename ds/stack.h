#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../basic.h"



typedef struct stack_t stack_t ;



#define             stack_init(ARR, SIZE, CAPACITY)     __impl_stack_init((void **)(ARR), (SIZE), CAPACITY)

#define             stack_push(PSTACK, ELEM)            __impl_stack_push((PSTACK), ELEM, &(ELEM), sizeof(ELEM))

//NOTE: if the value exceeds 8 bytes in size, the stack returns a buffer holding the value 
//      so to be copied to someother place else it just returns the value itself if its 
//      less than 8 bytes
void *              stack_pop(stack_t *);
void                stack_delete(stack_t *);

void                stack_destroy(stack_t *);

#define             stack_is_empty(pstack)              ((pstack)->top == -1 ? true : false)
#define             stack_is_full(pstack)               ((pstack)->top == ((pstack)->capacity - 1) ? true : false)

void                stack_print(stack_t *stack, void (*print_elem)(void *));

#define             stack_loop(PSTACK, VAR)             for(i32 VAR = 0; VAR <= (PSTACK)->top; VAR++)











#ifndef IGNORE_STACK_IMPLEMENTATION

struct stack_t {

    void **array;
    i32 top;
    u32 capacity;
    u32 size_of_each_elem;

    // Buffer that stores the get values from queue_get calls instead of having to pass 
    // a buffer to have it store the output
    void *buffer;

};


static inline stack_t __impl_stack_init(void **array, u32 arr_size_in_bytes, u32 capacity)
{
    if (array == NULL) eprint("stack_init: array argument is null");
    if(arr_size_in_bytes == 8 && capacity == 0) eprint("passed in a pointer not a static array");

    // Checks if passed in array is actually an array or a pointer

    memset(array, 0, arr_size_in_bytes);

    return (stack_t) {
        .array = array,
        .top = -1,
        .capacity = capacity,
        .size_of_each_elem = arr_size_in_bytes / capacity,
        .buffer = (void *)calloc(1, arr_size_in_bytes/ capacity)
    };
}


static inline void __impl_stack_push(stack_t *stack, void *elem, void *elem_ref, u64 elem_size)
{
    // NOTE: since sizeof void * is 8 bytes and maximum size of a primitive data type 
    // available in c is also 8 bytes, having the array hold it by value is enough,
    // but if the value passed exceeds 8 bytes, a deep copy of the value is made 
    // into the array- hoping the array has enough space to accomodate and not overflow.

    if (stack == NULL)                          eprint("stack_push: stack argument is null");
    if (elem_ref == NULL && elem == NULL)       eprint("stack_push: elem argument is null");
    if (stack->top == (i64)stack->capacity-1)   eprint("stack_push: overflow");


    if (elem_size <= 8) {

        stack->array[++stack->top] = elem;

    } else {

        u8 *arr = (u8 *)stack->array + (++stack->top * elem_size); 
        memcpy(arr, elem_ref, elem_size);

    }
}

void * stack_pop(stack_t *stack)
{
    if (stack == NULL) eprint("stack_push: stack argument is null");
    if (stack->top == -1) return NULL;

    if (stack->size_of_each_elem <= 8)
    {
        void *elem = stack->array[stack->top];
        stack->top--;
        return elem;
    }

    void *elem_pos = stack->array + stack->top * stack->size_of_each_elem;
    stack->top--;

    memcpy(stack->buffer, elem_pos, stack->size_of_each_elem);

    return stack->buffer;
}

void stack_delete(stack_t *stack)
{
    if (stack == NULL) eprint("stack_push: stack argument is null");
    
    if (stack->top == -1) {
        eprint("stack_push: underflow");
    }

    if (stack->size_of_each_elem <= 8)
        stack->array[stack->top] = NULL;
    else
        memset(stack->array + stack->top * stack->size_of_each_elem, 0, stack->size_of_each_elem);

    stack->top--;
}


void stack_print(stack_t *stack, void (*print_elem)(void *))
{
    if (stack == NULL) eprint("stack_print: stack argument is null");

    if(stack->top == -1) printf("stack_print: stack is empty");

    for (int i = 0; i <= stack->top; i++) print_elem(stack->array[i]);
    printf("\n");
}

void stack_destroy(stack_t *stack)
{
    assert(stack);

    free(stack->buffer);
    stack->buffer = NULL;

    stack->array = NULL;
    stack->top = -1;
    stack->capacity = 0;
    stack->size_of_each_elem = 0;
}

#endif 

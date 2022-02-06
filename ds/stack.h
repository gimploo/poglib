#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../basic.h"



typedef struct stack_t stack_t ;



#define             stack_init(CAPACITY, TYPE)          __impl_stack_init((CAPACITY), #TYPE, sizeof(TYPE))

#define             stack_push(PSTACK, ELEM)            __impl_stack_push((PSTACK), &(ELEM), sizeof(ELEM))

void *              stack_pop(stack_t *);

void                stack_destroy(stack_t *);

void                stack_print(stack_t *stack, void (*print_elem)(void *));

#define             stack_is_empty(pstack)              ((pstack)->top == -1 ? true : false)
#define             stack_is_full(pstack)               ((pstack)->top == ((pstack)->capacity - 1) ? true : false)












#ifndef IGNORE_STACK_IMPLEMENTATION

struct stack_t {

    u64 len;
    u8 *array;
    i64 top;
    u64 capacity;
    u64 elem_size;

    // This variable checks if the list is a list of pointers 
    bool __are_values_pointers;

};


stack_t __impl_stack_init(u64 capacity, const char *elem_name, u64 elem_size)
{
    bool flag = false;
    if (elem_name[strlen(elem_name) - 1] == '*') flag = true;

    stack_t o = {
        .len = 0,
        .array = (u8 *)calloc(capacity, elem_size),
        .top = -1,
        .capacity = capacity,
        .elem_size = elem_size,
        .__are_values_pointers = flag
    };

    return o;
}


void __impl_stack_push(stack_t *stack, void *elem_ref, u64 elem_size)
{
    // NOTE: since sizeof void * is 8 bytes and maximum size of a primitive data type 
    // available in c is also 8 bytes, having the array hold it by value is enough,
    // but if the value passed exceeds 8 bytes, a deep copy of the value is made 
    // into the array- hoping the array has enough space to accomodate and not overflow.

    if (stack == NULL)                          eprint("stack_push: stack argument is null");
    if (elem_ref == NULL )                      eprint("stack_push: elem argument is null");
    if (stack->top == (i64)stack->capacity-1)   eprint("stack_push: overflow");

    if (elem_size != stack->elem_size) eprint("trying to push a value of size %lu to slot of size %lu", elem_size, stack->elem_size);

    u8 *arr = (u8 *)stack->array + (++stack->top * elem_size); 
    memcpy(arr, elem_ref, elem_size);

    stack->len = stack->top + 1;

}

void * stack_pop(stack_t *stack)
{
    if (stack == NULL) eprint("stack argument is null");
    if (stack->top == -1) eprint("underflow");

    u8 *elem_pos = (u8 *)stack->array + stack->top * stack->elem_size;
    stack->len = --stack->top - 1;

    if (stack->__are_values_pointers) return *(void **)elem_pos;

    return elem_pos;
}


void stack_print(stack_t *stack, void (*print_elem)(void *))
{
    if (stack == NULL) eprint("stack_print: stack argument is null");

    if(stack->top == -1) printf("stack_print: stack is empty");

    printf("\nSTACK ----------------------\n");
    for (int i = stack->top; i > -1; i--) {
        printf("\t");
        print_elem(stack->array + i * stack->elem_size);
        printf("\n");
    }
    printf("---------------------------\n");
}

void stack_destroy(stack_t *stack)
{
    assert(stack);

    free(stack->array);
    stack->array = NULL;

    stack->array = NULL;
    stack->top = -1;
    stack->capacity = 0;
    stack->elem_size = 0;
    stack->len = 0;
}

#endif 

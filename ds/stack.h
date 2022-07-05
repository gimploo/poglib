#pragma once
#include "../basic.h"

/*=============================================================================
                            - STACK DATA STRUCTURE -
=============================================================================*/

typedef struct stack_t {

    u64     len;
    u8      *__array;
    i64     __top;
    u64     __capacity;
    u64     __elem_size;
    char    __elem_type[16];
    bool    __are_values_pointers;     // This variable checks if the list is a list of pointers 
                                       
} stack_t ;


#define             stack_init(CAPACITY, TYPE)          __impl_stack_init((CAPACITY), #TYPE, sizeof(TYPE))
#define             stack_push(PSTACK, ELEM)            __impl_stack_push((PSTACK), &(ELEM), sizeof(ELEM))
void *              stack_pop(stack_t *);
void                stack_destroy(stack_t *);
void                stack_print(stack_t *stack, void (*print_elem)(void *));
#define             stack_is_empty(pstack)              ((pstack)->top == -1 ? true : false)
#define             stack_is_full(pstack)               ((pstack)->top == ((pstack)->capacity - 1) ? true : false)

#ifndef IGNORE_STACK_IMPLEMENTATION
/*-----------------------------------------------------------------------------
                                IMPLEMENTATION
-----------------------------------------------------------------------------*/

stack_t __impl_stack_init(u64 capacity, const char *elem_type, u64 elem_size)
{
    bool flag = false;
    u32 len = strlen(elem_type);
    if (elem_type[len] > 15) eprint("variable type is too big, exceeded the 16 limit threshold\n");
    if (elem_type[len - 1] == '*') flag = true;

    stack_t o = {
        .len                    = 0,
        .__array                = (u8 *)calloc(capacity, elem_size),
        .__top                  = -1,
        .__capacity             = capacity,
        .__elem_size            = elem_size,
        .__elem_type            = {0},
        .__are_values_pointers  = flag
    };

    if (!flag)  memcpy(o.__elem_type, elem_type, 15);
    else        memcpy(o.__elem_type, elem_type, len - 1);

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
    if (stack->__top == (i64)stack->__capacity-1)   eprint("stack_push: overflow");

    if (elem_size != stack->__elem_size) eprint("trying to push a value of size %lu to slot of size %lu", elem_size, stack->__elem_size);

    u8 *arr = (u8 *)stack->__array + (++stack->__top * elem_size); 
    memcpy(arr, elem_ref, elem_size);

    stack->len = stack->__top + 1;

}

void * stack_pop(stack_t *stack)
{
    if (stack == NULL) eprint("stack argument is null");
    if (stack->__top == -1) eprint("underflow");

    u8 *elem_pos = (u8 *)stack->__array + stack->__top * stack->__elem_size;
    stack->len = --stack->__top - 1;

    if (stack->__are_values_pointers) return *(void **)elem_pos;

    return elem_pos;
}


void stack_print(stack_t *stack, void (*print_elem)(void *))
{
    if (stack == NULL) eprint("stack_print: stack argument is null");

    if(stack->__top == -1) printf("stack_print: stack is empty");

    printf("\nSTACK ----------------------\n");
    for (int i = stack->__top; i > -1; i--) {
        printf("\t");
        print_elem(stack->__array + i * stack->__elem_size);
        printf("\n");
    }
    printf("---------------------------\n");
}

void stack_destroy(stack_t *stack)
{
    assert(stack);

    free(stack->__array);
    stack->__array = NULL;

    stack->__array = NULL;
    stack->__top = -1;
    stack->__capacity = 0;
    stack->__elem_size = 0;
    stack->len = 0;
}

#endif 

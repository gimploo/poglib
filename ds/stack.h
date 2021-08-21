#ifndef __MY_STACK_H__
#define __MY_STACK_H__

#include <stdio.h>
#include <stdlib.h>

#include "../basic.h"

typedef struct stack_t stack_t;
struct stack_t {
    void **array;
    i64 top;
    u64 capacity;
};

static inline void stack_dump(stack_t *stack)
{
    if (stack == NULL) eprint("stack_dump: stack argument is null");

    fprintf(stderr, "[ERROR] DUMPING STACK\n");
    for (u64 i = 0; i < stack->capacity; i++)
        fprintf(stdout, "%p ", stack->array[i]);
    printf("\n");
}

#define stack_is_empty(pstack) ((pstack)->top == -1 ? true : false)


static inline stack_t stack_init(void **array, size_t capacity)
{
    if (array == NULL) eprint("stack_init: array argument is null");
    if (capacity == 0) eprint("stack_init: capacity not greater than zero");

    for (u64 i = 0; i < capacity; i++) array[i] = NULL;

    return (stack_t) {
        .array = array,
        .top = -1,
        .capacity = capacity
    };
}


static void __impl_stack_push(stack_t *stack, void *elem_ref, u64 elem_size)
{

    // NOTE: since sizeof void * is 8 bytes and maximum size of a primitive data type 
    // available in c is also 8 bytes, having the array hold it by value is enough,
    // but if the value passed exceeds 8 bytes, a deep copy of the value is made 
    // into the array- hoping the array has enough space to accomodate and not overflow.

    if (stack == NULL) eprint("stack_push: stack argument is null");
    if (elem_ref == NULL) eprint("stack_push: elem argument is null");

    if (stack->top == (i64)stack->capacity-1) {
        stack_dump(stack);
        eprint("stack_push: overflow");
    }

    if (elem_size <= 8) {

        memcpy(&stack->array[++stack->top], elem_ref, elem_size);

    } else {

        u8 *arr = (u8 *)stack->array; 
        u64 offset = (++stack->top * elem_size);
        memcpy((arr + offset), elem_ref, elem_size);

    }

}

#define stack_push(pstack, elem) __impl_stack_push((pstack), &(elem), sizeof (elem) )


static inline void * stack_pop(stack_t *stack)
{
    if (stack == NULL) eprint("stack_push: stack argument is null");
    if (stack->top == -1) return NULL;

    void *elem = stack->array[stack->top];

    stack->array[stack->top] = NULL;
    stack->top--;

    return elem;
}

static inline void stack_delete(stack_t *stack)
{
    if (stack == NULL) eprint("stack_push: stack argument is null");
    
    if (stack->top == -1) {
        stack_dump(stack);
        eprint("stack_push: underflow");
    }

    stack->array[stack->top] = NULL;
    stack->top--;
}

#define for_i_in_stack(pstack) for(int i = 0; i <= (pstack)->top; i++)

static inline void stack_print(stack_t *stack, void (*print_elem)(void *))
{
    if (stack == NULL) eprint("stack_print: stack argument is null");

    if(stack->top == -1) printf("stack_print: stack is empty");

    for (int i = 0; i <= stack->top; i++) print_elem(stack->array[i]);
    printf("\n");
}


#endif //__MY_STACK_H__

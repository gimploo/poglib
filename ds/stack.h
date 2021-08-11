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

static inline bool stack_is_empty(stack_t *stack)
{
    if (stack == NULL) eprint("stack_is_empty: stack argument is null");

    return stack->top == -1 ? true : false; 
}


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

static inline void stack_push(stack_t *stack, void *elem)
{
    if (stack == NULL) eprint("stack_push: stack argument is null");
    if (elem == NULL) eprint("stack_push: elem argument is null");

    if (stack->top == stack->capacity-1) {
        stack_dump(stack);
        eprint("stack_push: overflow");
    }
    
    stack->array[++stack->top] = elem;
}

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

static inline void stack_print(stack_t *stack, void (*print_elem)(void *))
{
    if (stack == NULL) eprint("stack_print: stack argument is null");

    if(stack->top == -1) printf("stack_print: stack is empty");

    for (int i = 0; i <= stack->top; i++) print_elem(stack->array[i]);
    printf("\n");
}


#endif //__MY_STACK_H__

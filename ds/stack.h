#ifndef __MY_STACK_H__
#define __MY_STACK_H__

#include <stdio.h>
#include <stdlib.h>

#include "../basic.h"

/*============================================================
 // Stack data structure
============================================================*/

typedef struct stack_t {

    void **array;
    i64 top;
    u64 capacity;
    u64 size_of_each_elem;

} stack_t;


/*---------------------------------------------------------------
 // Declaration
---------------------------------------------------------------*/

//The 2 init methods are for two different cases, if your planning on passing a static array use the first one and 
//if your planning on passing a dynamic array (i.e. malloced pointers) use the second
//NOTE:(macro)      stack_static_array_init(array[], array_capacity) -> stack_t
//                  (or)
//NOTE:(macro)      stack_dynamic_array_init(array[], array_size_in_bytes, array_capacity) -> stack_t

//NOTE:(macro)      stack_is_empty(stack_t *) -> bool
//NOTE:(macro)      stack_is_full(stack_t *) -> bool
void                stack_print(stack_t *stack, void (*print_elem)(void *));

//NOTE:(macro)      stack_push(stack_t *, elem) -> void
void *              stack_pop(stack_t *);
void                stack_delete(stack_t *);


/*---------------------------------------------------------------
 // Implementation
---------------------------------------------------------------*/

#define stack_is_empty(pstack) ((pstack)->top == -1 ? true : false)
#define stack_is_full(pstack)  ((pstack)->top == ((pstack)->capacity - 1) ? true : false)


static inline stack_t __impl_stack_init(void **array, size_t arr_size_in_bytes, size_t capacity)
{
    if (array == NULL) eprint("stack_init: array argument is null");
    if(arr_size_in_bytes == 8 && capacity == 0) eprint("passed in a pointer not a static array");

    // Checks if passed in array is actually an array or a pointer

    memset(array, 0, arr_size_in_bytes);

    return (stack_t) {
        .array = array,
        .top = -1,
        .capacity = capacity,
        .size_of_each_elem = arr_size_in_bytes / capacity
    };
}
#define stack_static_array_init(arr, capacity) __impl_stack_init((void **)(arr), sizeof(arr), capacity)
#define stack_dynamic_array_init(arr, bytes, capacity) __impl_stack_init((void **)(arr), (bytes), capacity)


static void __impl_stack_push(stack_t *stack, void *elem_ref, u64 elem_size)
{

    // NOTE: since sizeof void * is 8 bytes and maximum size of a primitive data type 
    // available in c is also 8 bytes, having the array hold it by value is enough,
    // but if the value passed exceeds 8 bytes, a deep copy of the value is made 
    // into the array- hoping the array has enough space to accomodate and not overflow.

    if (stack == NULL) eprint("stack_push: stack argument is null");
    if (elem_ref == NULL) eprint("stack_push: elem argument is null");

    if (stack->top == (i64)stack->capacity-1) {
        eprint("stack_push: overflow");
    }

    if (elem_size <= 8) {

        memcpy(&stack->array[++stack->top], elem_ref, elem_size);

    } else {

        u8 *arr = (u8 *)stack->array + (++stack->top * elem_size); 
        memcpy(arr, elem_ref, elem_size);

    }

}

#define stack_push(pstack, elem) __impl_stack_push((pstack), &(elem), sizeof(elem))


void * stack_pop(stack_t *stack)
{
    if (stack == NULL) eprint("stack_push: stack argument is null");
    if (stack->top == -1) return NULL;

    void *elem = (stack->array + (stack->top * stack->size_of_each_elem));

    *(stack->array + (stack->top * stack->size_of_each_elem)) = NULL;
    stack->top--;

    return elem;
}

void stack_delete(stack_t *stack)
{
    if (stack == NULL) eprint("stack_push: stack argument is null");
    
    if (stack->top == -1) {
        eprint("stack_push: underflow");
    }

    stack->array[stack->top] = NULL;
    stack->top--;
}

#define for_i_in_stack(pstack) for(int i = 0; i <= (pstack)->top; i++)

void stack_print(stack_t *stack, void (*print_elem)(void *))
{
    if (stack == NULL) eprint("stack_print: stack argument is null");

    if(stack->top == -1) printf("stack_print: stack is empty");

    for (int i = 0; i <= stack->top; i++) print_elem(stack->array[i]);
    printf("\n");
}


#endif //__MY_STACK_H__

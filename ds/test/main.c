#include <stdio.h>
#include <stdlib.h>

#include "../stack.h"



void print_int(void *x)
{
    printf("%i ", (int)x);
}

struct foo_t {
    char* data; 
    size_t size;
};

void print_foo(void *x)
{
    struct foo_t *foo = (struct foo_t *) x;
    printf("%s ", foo->data);
    printf("%li ",foo->size);
}

int main(void)
{
    struct foo_t foo = {"hello world", strlen("hello world")};
    struct foo_t bar = {"foobar", strlen("foobar")};

    struct foo_t arr[10];
    stack_t stack = stack_init(arr, 10);
    stack_dump(&stack);

    stack_push(&stack, &foo);
    stack_push(&stack, &foo);
    stack_push(&stack, &foo);
    stack_push(&stack, &foo);
    stack_push(&stack, &foo);
    stack_push(&stack, &foo);
    stack_push(&stack, &foo);
    stack_push(&stack, &foo);
    stack_print(&stack, print_foo);
    stack_push(&stack, &bar);
    stack_print(&stack, print_foo);
    stack_pop(&stack);
    stack_print(&stack, print_foo);

    stack_print(&stack, print_foo);
    stack_dump(&stack);

    return 0;
}


int oldmain(void)
{
    int **array = (int **) malloc(sizeof(int *) * 5);
    stack_t stack = stack_init(array, 5); 

    stack_dump(&stack);

    stack_push(&stack ,1);
    stack_dump(&stack);
    stack_delete(&stack);
    stack_dump(&stack);
    stack_push(&stack ,2);
    stack_push(&stack ,3);
    stack_push(&stack ,4);
    stack_pop(&stack);
    stack_push(&stack ,5);

    stack_print(&stack, print_int);

    stack_dump(&stack);
    
    free(array);
    return 0;
}

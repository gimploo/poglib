#include <stdio.h>
#include <stdlib.h>

#include "../stack.h"


void print_int(void *x)
{
    printf("%i ", (int)x);
}

void print_long(void *x)
{
    printf("%lli ", (long long)x);
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
    u64 foo[10];
    stack_t stack01 = stack_init(foo, 10);

    stack_print(&stack01, print_int);

    for (u64 i = 1; i <= 10; i++) stack_push(&stack01, (void *)i);

    stack_print(&stack01, print_int);

    stack_dump(&stack01);

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

#include <stdio.h>
#include <stdlib.h>

#include "../stack.h"



void print_int(void *x)
{
    printf("%i ", (int)x);
}

int main(void)
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

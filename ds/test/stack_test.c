#include <stdio.h>
#include <stdlib.h>

#include "../stack.h"
#include "../../math/la.h"


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

void print_vec4f(void *x)
{
    vec4f_t *vec = (vec4f_t *)x;
    printf(VEC4F_FMT"\n", VEC4F_ARG(vec));
}

int oldmain(void)
{
    int foo[10];

    stack_t stack = stack_init(foo, 10);

    stack_dump(&stack);
    for (int i = 1; i <= 10; i++)
        stack_push(&stack, i);
    stack_dump(&stack);

    stack_print(&stack, print_int);

}


int main(void)
{
    void ** foo[10] = {0};

    vec4f_t vecs[10] = {0}; 

    stack_t stack01 = stack_init(foo, 10);
    stack_t stack02 = stack_init(vecs, 10);
    stack_dump(&stack01);

    vec4f_t data[10] = {

        vec4f(0.0f),
        vec4f(1.0f),
        vec4f(2.0f),
        vec4f(3.0f),
        vec4f(4.0f),
        vec4f(5.0f),
        vec4f(6.0f),
        vec4f(7.0f),
        vec4f(8.0f),
        vec4f(9.0f),
    }; 

    for (int i = 0; i < 10; i++)
    {
        /*stack_push(&stack01, &data[i], sizeof(data[i]));*/
        stack_push(&stack02, data[i]);
    }
    /*stack_print(&stack01, print_vec4f);*/
    printf("\n");

    /*for_i_in_stack(&stack01)*/
    {
        /*vec4f_t *vec = ((vec4f_t *)stack01.array + i);*/
        /*printf(VEC4F_FMT"\n", VEC4F_ARG(vec));*/
    }
    /*printf("\n");*/

    for_i_in_stack(&stack02)
    {
        vec4f_t *vec = ((vec4f_t *)stack02.array + i);
        printf(VEC4F_FMT"\n", VEC4F_ARG(vec));
    }
    printf("\n");

    /*for (int i = 0; i < 10; i++)*/
        /*printf(VEC4F_FMT"\n", VEC4F_ARG(&vecs[i]));*/


    return 0;
}



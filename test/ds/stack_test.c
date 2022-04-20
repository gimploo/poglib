#include <stdio.h>
#include <stdlib.h>

#include "../stack.h"
#include "../../math/la.h"


void print_int(void *x)
{
    printf("%i ", *(int *)x);
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
    printf(VEC4F_FMT, VEC4F_ARG(vec));
}

int test01(void)
{
    stack_t stack = stack_init(10, int);

    for (int i = 0; i < 10; i++)
        stack_push(&stack, i);

    stack_print(&stack, print_int);

    stack_destroy(&stack);

    return 0;
}


int test02(void)
{
    stack_t stack01 = stack_init(10, void **);
    stack_t stack02 = stack_init(10, vec4f_t );

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

    for(int i = stack02.top; i > -1 ;i--)
    {
        vec4f_t *vec = ((vec4f_t *)stack02.array + i);
        printf(VEC4F_FMT"\n", VEC4F_ARG(vec));
    }
    printf("\n");

    stack_pop(&stack02);

    /*for (int i = 0; i < 10; i++)*/
        /*printf(VEC4F_FMT"\n", VEC4F_ARG(&vecs[i]));*/

    stack_destroy(&stack01);
    stack_destroy(&stack02);

    return 0;
}

typedef struct bar {
    const char *label;
    int list[5];
} bar;

void print_bar(void *arg)
{
    bar *tmp = (bar *)arg;
    printf("%s [%i, %i, %i, %i, %i]" ,
            tmp->label, tmp->list[0], tmp->list[1], tmp->list[2], tmp->list[3], tmp->list[4] );
}

int test03(void)
{

    bar a = {
        .label = "a",
        .list = {1,2,3,4,5}
    };
    bar b = {
        .label = "b",
        .list = {6,7,8,9,10}
    };

    bar c = {
        .label = "c",
        .list = {11,12,13,14,15}
    };



    stack_t stack = stack_init(5, bar);


    stack_push(&stack, a);
    stack_push(&stack, b);
    stack_push(&stack, c);

    bar *tmp = stack_pop(&stack);
    print_bar(tmp);

    stack_print(&stack, print_bar);

    stack_destroy(&stack);

    return 0;
}

int main(void)
{
    printf("----------\n");
    printf("TESTS\n");
    printf("----------\n");

    printf("1. Test01\n");
    test01();
    printf("\n\n\n");
    printf("2. Test02\n");
    test02();
    printf("\n\n\n");
    printf("3. Test03\n");
    test03();
    printf("\n\n\n");

    return 0;
}



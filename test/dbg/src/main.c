#define DEBUG
#include <poglib/basic.h>
#include "./folder/foo.h"

int test01(void)
{
    dbg_init();

    int *array = malloc(sizeof(int) * 3);
    printf("array = %p\n", array);

    void *tmp = foo();

    array = realloc(array, sizeof(int) * 10);
    printf("array = %p\n", array);

    dbg_destroy();

    return 0;
}

int test02(void)
{
    dbg_init();

    int *array[20];

    for (int i = 0; i < 20; i++)
    {
        array[i] =  malloc(sizeof(int) * 5);
    }

    double *foo[20];
    for (int i = 0; i < 20; i++)
    {
        foo[i] =  calloc(sizeof(double) , 1);
    }

    for (int i = 0; i < 20; i++)
    {
        foo[i] =  realloc(foo[i], 5 * sizeof(double));
    }
    for (int i = 0; i < 20; i++)
    {
        free(foo[i]);
    }

    for (int i = 0; i < 20; i++)
    {
        array[i] =  realloc(array[i], sizeof(int) * 5);
    }

    for (int i = 0; i < 20; i++)
    {
        free(array[i]);
    }

    dbg_destroy();
    return 0;
}

int main(void)
{
    printf("Test 01\n");
    /*test01();*/
    printf("Test 02\n");
    test02();
    return 0;
}

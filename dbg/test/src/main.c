#include "../../dbg.h"

dbg_t debug;

int main(void)
{
    dbg_init(&debug, "./tmp.txt");

    int *array[20];

    for (int i = 0; i < 20; i++)
    {
        array[i] =  malloc(sizeof(int) * 5);
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

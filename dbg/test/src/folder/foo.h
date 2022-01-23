#include <stdio.h>



void * foo(void)
{
    return malloc(sizeof(int) * 10);
}

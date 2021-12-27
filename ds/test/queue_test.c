#include <stdio.h>
#include ".././queue.h"

void print_int(void *elem)
{
    int value = (int)elem;
    printf("%i ", value);
}

int main(void)
{
    int array[10] = {0};
    queue_t queue = queue_init(array, sizeof(array), 10);

    int values[10] = {0,1,2,3,4,5,6,7,8,9};

    queue_print(&queue, print_int);

    for (int i = 0; i < 10; i++)
        queue_put(&queue, values[i]);

    queue_print(&queue, print_int);

    for (int i = 0; i < 10; i++)
    {
        queue_get(&queue);
        queue_print(&queue, print_int);
    }
   
    queue_destroy(&queue);

    return 0;
}

#include <stdio.h>
#include ".././queue.h"

void print_int(void *elem)
{
    printf("%i", (int )elem);
}

int main(void)
{
    int array[10] = {0};
    queue_t queue = queue_init(array, 10);

    queue_put(&queue, 1);
    queue_put(&queue, 2);
    queue_print(&queue, print_int);
    queue_put(&queue, 3);
    queue_put(&queue, 4);
    queue_put(&queue, 5);
    queue_put(&queue, 6);
    queue_put(&queue, 7);
    queue_put(&queue, 8);
    queue_put(&queue, 9);

    queue_get(&queue);
    queue_print(&queue, print_int);


    return 0;
}

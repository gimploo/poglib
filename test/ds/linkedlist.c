#include "../linkedlist.h"

void printint(void *arg)
{
    int value = *(int*)arg;
    printf("%i ", value);
}

bool compare(void *arg01, void *arg02)
{
    int x = *(int *)arg01;
    int y = *(int *)arg02;

    return (x == y);
}

int main(void)
{
    llist_t list = llist_init();

    for (int i = 0; i < 10; i++)
    {
        node_t *node = node_init(&i, sizeof(int));
        llist_append_node(&list, node);
    }

    int arr[3] = {9, 2, 0};

    llist_delete_node_by_value(&list, &arr[0], compare);
    llist_delete_node_by_value(&list, &arr[1], compare);
    llist_delete_node_by_value(&list, &arr[2], compare);

    llist_print(&list, printint);

    llist_destroy(&list);

    return 0;
}


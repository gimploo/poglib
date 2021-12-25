#include "../list.h"


void print_u8(void *arg)
{
    u8 num = (u8)arg;
    printf("%i ", num);
}


int main(void)
{
    list_t list = list_init(10);

    list_print(&list, print_u8);

    for (int i = 1; i < 12; i++)
    {
        list_append(&list, i * 10 + 1);
        list_print(&list, print_u8);
    }

    list_delete(&list, 0);
        list_print(&list, print_u8);
    list_delete(&list, 4);
        list_print(&list, print_u8);
    list_delete(&list, 6);
        list_print(&list, print_u8);
    list_delete(&list, 7);
        list_print(&list, print_u8);
    list_delete(&list, 10);



    list_print(&list, print_u8);

    list_free(&list);

    return 0;
}

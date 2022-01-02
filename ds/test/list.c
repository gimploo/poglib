#include "../list.h"




typedef struct foo {
    const char *label;
    int list[5];
} foo;


void print_foo(void *x)
{
    foo *tmp = (foo *) x;
    printf("%s ", tmp->label);
    for (int i = 0; i < 5; i++)
        printf(" %i",tmp->list[i]);
    printf("\n");
}

void print_pfoo(void *x)
{
    foo *tmp = *(foo **) x;
    printf("%s ", tmp->label);
    for (int i = 0; i < 5; i++)
        printf(" %i",tmp->list[i]);
    printf("\n");
}

list_t dummy(void)
{
    foo a = {
        .label = "a",
        .list = {1,2,3,4,5}
    };

    foo b = {
        .label = "b",
        .list = {6,7,8,9,10}
    };

    foo c = {
        .label = "c",
        .list = {11,12,13,14,15}
    };
    list_t list = list_init(3, foo);

    /*list_dump(&list);*/

    list_append(&list, a);
    list_append(&list, b);
    list_append(&list, c);
    list_append(&list, a);
    list_append(&list, b);
    list_append(&list, c);
    list_append(&list, a);
    list_append(&list, b);
    list_append(&list, c);
    return list;
}

int main(void)
{
    // Pointer

    foo a = {
        .label = "a",
        .list = {1,2,3,4,5}
    };

    foo b = {
        .label = "b",
        .list = {6,7,8,9,10}
    };

    foo c = {
        .label = "c",
        .list = {11,12,13,14,15}
    };

    foo *values[3] = { &a, &b, &c };
    list_t list = list_init(3, foo *);

    /*list_dump(&list);*/

    list_append(&list, values[0]);
    list_append(&list, values[1]);
    list_append(&list, values[2]);
    list_append(&list, values[0]);
    list_append(&list, values[1]);
    list_append(&list, values[2]);
    list_append(&list, values[0]);
    list_append(&list, values[1]);
    list_append(&list, values[2]);
    list_print(&list, print_pfoo);


    list_delete(&list, 0);
    list_delete(&list, 2);
    list_print(&list, print_pfoo);
    list_delete(&list, 2);
    list_print(&list, print_pfoo);
    list_delete(&list, 2);
    list_print(&list, print_pfoo);

    list_dump(&list);

    list_destroy(&list);

    return 0;
}

int oldmain(void)
{
    // Value

    list_t list = dummy();
    list_print(&list, print_foo);


    list_delete(&list, 0);
    list_delete(&list, 2);
    list_print(&list, print_foo);
    list_delete(&list, 2);
    list_print(&list, print_foo);
    list_delete(&list, 2);
    list_print(&list, print_foo);

    list_dump(&list);

    list_destroy(&list);

    return 0;
}




// Using Primitvie datattypes

void print_u32(void *arg)
{
    u32 num = *(u32 *)arg;
    printf("%i ", num);
}


int oldmain02(void)
{
    list_t list = list_init(10, u32);

    list_print(&list, print_u32);

    u32 values[10] = {1,2,3,4,5,6,7,8,9,10};


    for (int i = 0; i < 10; i++)
    {
        list_append(&list, values[i]);
        list_print(&list, print_u32);
    }

    list_delete(&list, 4);
        list_print(&list, print_u32);
    list_delete(&list, 0);
        list_print(&list, print_u32);
    list_delete(&list, 1);
        list_print(&list, print_u32);
    list_delete(&list, 6);
        list_print(&list, print_u32);
    
    list_dump(&list);




    list_destroy(&list);

    return 0;
}

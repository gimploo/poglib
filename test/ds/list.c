#define DEBUG
#include <poglib/basic.h>

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
    list_t list = list_init(foo);

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

int test05(void)
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
    list_t list = list_init(foo *);

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

int test01(void)
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

void print_str(void *x)
{
    str_t *y = (str_t *)x;

    str_print(y);
}

int test02(void)
{
    dbg_init();
    list_t list = list_init(str_t );

    str_t words[] = {
        str("yo"),
        str("bruh"),
        str("gucci"),
    };

    for (u32 i = 0; i < ARRAY_LEN(words); i++)
    {
        list_append(&list, words[i]);
    }

    list_print(&list, print_str);

    list_destroy(&list);

    dbg_destroy();
    return 0;
}


int test03(void)
{
    list_t list = list_init(u32);

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
    
    list_delete(&list, 0);
        list_print(&list, print_u32);

    list_delete(&list, 0);
        list_print(&list, print_u32);

    list_delete(&list, 0);
        list_print(&list, print_u32);

    list_delete(&list, 0);
        list_print(&list, print_u32);
    list_delete(&list, 0);
        list_print(&list, print_u32);
    list_delete(&list, 0);
        list_print(&list, print_u32);

    list_dump(&list);


    list_destroy(&list);

    return 0;
}

int test04(void)
{
    list_t list = list_init(list_t );

    for (int i = 0; i < 4; i++)
    {
        list_t tmp = list_init(foo);
        list_append(&list, tmp);
    }

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

    list_t *tmp = (list_t *)list_get_value(&list, 3);
    list_append(tmp, a);
    list_append(tmp, b);
    list_append(tmp, c);

    tmp = (list_t *)list_get_value(&list, 2);
    list_append(tmp, b);

    tmp = (list_t *)list_get_value(&list, 1);
    list_append(tmp, c);

    list_print(&list, list_dump);

    for (int i = 0; i < list.len; i++)
    {
        list_t *bar = (list_t *)list_get_value(&list, i);
        if (!bar->len) {
            printf("list %i list is empty\n", i);
            continue;
        }
        printf("list %i is \n",i);
        for (int j = 0; j < bar->len; j++)
        {
            foo *x = (foo *)list_get_value(bar, j);
            print_foo(x);

        }
    }

    printf("\n\n");

    list_delete(&list, 0);
    for (int i = 0; i < list.len; i++)
    {
        list_t *bar = (list_t *)list_get_value(&list, i);
        if (!bar->len) {
            printf("list %i list is empty\n", i);
            continue;
        }
        printf("list %i is \n",i);
        for (int j = 0; j < bar->len; j++)
        {
            foo *x = (foo *)list_get_value(bar, j);
            print_foo(x);

        }
    }


    for (int i = 0; i < list.len; i++)
    {
        list_t *tmp = (list_t *)list_get_value(&list, i);
        list_destroy(tmp);
    }
    list_destroy(&list);

    return 0;
}

void test06()
{
    list_t list = list_init(u32);

    int values[] = { 1,2,3,4,5,6,7,8,9,10};
    int *ptrs[] = {&values[0], &values[1], &values[2], &values[3], &values[4], &values[5] };
    for (u32 i = 0; i < 6; i++)
        list_append(&list, i);

    list_iterator(&list, elem) {
        printf("%i ", *(u32 *)elem);
    }
    printf("\n");

    /*list_delete(&list, 3);*/
    list_delete(&list, 5);
    /*list_delete(&list, 3);*/
    /*list_delete(&list, 3);*/

    list_iterator(&list, elem02) {
        printf("%i ", *(u32 *)elem02);
    }

    list_destroy(&list);
};

int main(void)
{
    dbg_init();
    test01();
    test02();
    test03();
    test04();
    test06();

    dbg_destroy();
    return 0;
};

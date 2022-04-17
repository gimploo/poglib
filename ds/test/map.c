#include "./../map.h"

void print_int(void *arg)
{
    printf("%i ", *(int *)arg);
}

void print_str(void *arg)
{
    printf("%s ", (char *)arg);
}

void test01(void)
{
    int values[] = {1,2,3,4,5,6,7,8,9,10,11};
    map_t map = map_init(10, u32);
    int *t1 = map_insert(&map, "bruh", values[2]);
    int *t2 = map_insert(&map, "jaguar", values[0]);
    int *t3 = map_insert(&map, "cat", values[1]);

    printf("%i %i %i\n", *t1, *t2, *t3);
    /*map_delete(&map, "jaguar");*/
    /*map_delete(&map, "cat");*/
    /*map_delete(&map, "bruh");*/

    list_print(&map.__keys, print_str);
    hashtable_print(&map.__values, print_int);

    int *r1 = map_get_value(&map, "cat");
    printf("(%i) \n", *r1);

    int j = 0;

    map_iterator(&map, juice) {
        printf("%i \n", *(u32 *)juice);
    }

    printf("%i ", *(u32 *)map_get_value_at_index(&map, 0));
    printf("%i ", *(u32 *)map_get_value_at_index(&map, 1));
    printf("%i ", *(u32 *)map_get_value_at_index(&map, 2));

    map_destroy(&map);
}

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

void test02(void)
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

    foo* ptrs[] = {&a, &b, &c};

    map_t map = map_init(10, foo *);
    map_insert(&map, "foo_b", ptrs[1]);
    map_insert(&map, "foo_a", ptrs[0]);
    map_insert(&map, "foo_c", ptrs[2]);

    foo *tmp[] = {
        map_get_value(&map, "foo_b"),
        map_get_value(&map, "foo_c"),
        map_get_value(&map, "foo_a"),
    };

    map_iterator(&map, elem) {
        print_foo(elem);
    }

    for (u32 i = 0; i < 3; i++)
        print_foo(tmp[i]);

    map_destroy(&map);

}

void foo_add_to_map(map_t *map)
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

    map_insert(map, a.label, a);
    map_insert(map, b.label, b);
    map_insert(map, c.label, c);
}

void test03()
{
    map_t map = map_init(10, foo);

    foo_add_to_map(&map);

    map_iterator(&map, stuff) {
        print_foo(stuff);
    }

    map_destroy(&map);
}


int main(void)
{
    printf("TEST01\n");
    test01();
    printf("TEST02\n");
    test02();
    printf("TEST03\n");
    test03();

    return 0;
}

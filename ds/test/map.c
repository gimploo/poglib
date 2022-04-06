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
    map_insert(&map, "jaguar", values[0]);
    map_insert(&map, "cat", values[1]);
    map_insert(&map, "bruh", values[2]);
    /*map_delete(&map, "jaguar");*/
    /*map_delete(&map, "cat");*/
    /*map_delete(&map, "bruh");*/

    hashtable_print(&map.__values, print_int);
    list_print(&map.__keys, print_str);
    map_destroy(&map);
}

void test02(void)
{
}


int main(void)
{
    test01();

    return 0;
}

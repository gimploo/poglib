#include <stdio.h>
#include <poglib/ds.h>


void print_str(void *arg)
{
    str_t *str = (str_t *)arg;
    printf(STR_FMT" ", STR_ARG(str));
}

int test01(void)
{
    dbg_init();

    str_t names[5] = {
        str("asldkfjas"),
        str("oqoewiruqweyr"),
        str(",zxmcn,mn"),
        str("9823weuiua"),
        str("][yopi.,m;lhj;hl]")
    };


    slotarray_t table = slotarray_init(100, str_t );

    for (int i = 0; i < 5; i++)
    {
        slotarray_insert(&table, i, names[i]);
        slotarray_print(&table, print_str);
    }
    /*slotarray_dump(&table);*/

    for (int i = 0; i < 5; i++)
    {
        slotarray_delete(&table, i);
        slotarray_print(&table, print_str);
    }

    /*slotarray_print(&table, print_str);*/
    slotarray_dump(&table);

    slotarray_destroy(&table);

    dbg_destroy();
    return 0;
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

void insert_foo_in_table_lul(slotarray_t *table)
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

    slotarray_insert(table, 0, a);
    slotarray_insert(table, 1, b);
    slotarray_insert(table, 2, c);
}

void test03(void)
{
    slotarray_t table = slotarray_init(10, foo);

    insert_foo_in_table_lul(&table);

    /*slotarray_print(&table, print_foo);*/
    slotarray_iterator(&table, iter) {
        foo *tmp = (foo *)iter;
        print_foo(tmp);
    }
    slotarray_dump(&table);

    slotarray_destroy(&table);
}

int test02(void)
{
    str_t name = str("gokul");
    u32 x = hash_cstr(name.buf, name.len);

    printf("string: "STR_FMT"\n", STR_ARG(&name));
    printf("index: %i\n", x % 10);

    str_t lol = str("gikul");
    u32 y = hash_cstr(lol.buf, lol.len);
    printf("string: "STR_FMT"\n", STR_ARG(&lol));
    printf("index: %i \n", y % 10);


    str_t juice = str("juice");
    u32 z = hash_cstr(juice.buf, lol.len);
    printf("string: "STR_FMT"\n", STR_ARG(&juice));
    printf("index: %i \n", z % 10);

    for (int i = 0; i < 100; i++)
    {
        char buff[4] = {0}; 
        snprintf(buff, sizeof(buff), "%i", i);
        u32 z = hash_cstr(buff, strlen(buff));
        printf("key: %s\n", buff);
        printf("index: %i \n", z % 100);
    }

    return 0;
}

int main(void)
{
    printf("test01\n");
    test01();
    printf("test02\n");
    test02();
    printf("test03\n");
    test03();
    return 0;
}

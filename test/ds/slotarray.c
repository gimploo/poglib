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


    slot_t table = slot_init(100, str_t );

    for (int i = 0; i < 5; i++)
    {
        slot_insert(&table, i * 4, names[i]);
    }

    slot_iterator(&table, iter) {
        str_t *name = (str_t *)iter;
        print_str(name);
        printf("\n");
    }

    for (int i = 0; i < 5; i++)
    {
        slot_delete(&table, i);
    }

    /*slot_print(&table, print_str);*/
    /*slot_dump(&table);*/

    slot_destroy(&table);

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

void insert_foo_in_table_lul(slot_t *table)
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

    slot_insert(table, 0, a);
    slot_insert(table, 1, b);
    slot_insert(table, 2, c);
}

void test03(void)
{
    slot_t table = slot_init(10, foo);

    insert_foo_in_table_lul(&table);

    /*slot_print(&table, print_foo);*/
    slot_iterator(&table, iter) {
        foo *tmp = (foo *)iter;
        print_foo(tmp);
    }
    slot_dump(&table);

    slot_destroy(&table);
}

int main(void)
{
    printf("test01\n");
    test01();
    printf("test03\n");
    /*test03();*/
    return 0;
}

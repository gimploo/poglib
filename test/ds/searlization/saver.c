#include <poglib/ds.h>

typedef struct foo {
    const char label[16];
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
void insert_foo_in_table_lul(void *table)
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

    /*hashtable_insert(table, a.label, a);*/
    /*hashtable_insert(table, b.label, b);*/
    /*hashtable_insert(table, c.label, c);*/
    slot_insert(table, 0, a);
    slot_insert(table, 1, b);
    slot_insert(table, 2, c);
}

void test02(void)
{
    slot_t table = slot_init(10, foo);

    insert_foo_in_table_lul(&table);

    slot_print(&table, print_foo);
    slot_dump(&table);

    file_t *input = file_init("config", "wb");
    file_save_slot(input, table);
    file_destroy(input);

    slot_destroy(&table);

}

void test01(void)
{
    hashtable_t table = hashtable_init(10, foo);

    insert_foo_in_table_lul(&table);

    hashtable_print(&table, print_foo);
    hashtable_dump(&table);

    file_t *input = file_init("config", "wb");
    file_save_hashtable(input, table);
    file_destroy(input);

    hashtable_destroy(&table);

}


int main(void)
{
    /*test01();*/
    test02();
    return 0;
}

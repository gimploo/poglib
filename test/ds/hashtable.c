#include <poglib/ds.h>


void print_str(void *arg)
{
    str_t *str = (str_t *)arg;
    printf(STR_FMT" ", STR_ARG(str));
}

int test01(void)
{
    str_t names[5] = {
        str("asldkfjas"),
        str("oqoewiruqweyr"),
        str(",zxmcn,mn"),
        str("9823weuiua"),
        str("][yopi.,m;lhj;hl]")
    };

    const char *keys[5] = {
        "jo",
        "jomama",
        "sigma",
        "deez",
        "forsencd"
    };

    hashtable_t table = hashtable_init(100, str_t );

    for (int i = 0; i < 5; i++)
    {
        hashtable_insert(&table, keys[i], names[i]);
        hashtable_print(&table, print_str);
    }
    /*hashtable_dump(&table);*/

    for (int i = 0; i < 5; i++)
    {
        hashtable_delete(&table, keys[i]);
        hashtable_print(&table, print_str);
    }

    /*hashtable_print(&table, print_str);*/
    hashtable_dump(&table);

    hashtable_destroy(&table);

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

void insert_foo_in_table_lul(hashtable_t *table)
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

    hashtable_insert(table, a.label, a);
    hashtable_insert(table, b.label, b);
    hashtable_insert(table, c.label, c);
}

void test03(void)
{
    hashtable_t table = hashtable_init(10, foo);

    insert_foo_in_table_lul(&table);

    hashtable_print(&table, print_foo);
    hashtable_dump(&table);

    hashtable_destroy(&table);
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
    /*printf("test01\n");*/
    /*test01();*/
    /*printf("test02\n");*/
    /*test02();*/
    /*printf("test03\n");*/
    /*test03();*/
    return 0;
}

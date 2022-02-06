#include <stdio.h>
#include "../hashtable.h"


void print_str(void *arg)
{
    str_t *str = (str_t *)arg;
    printf(STR_FMT" ", STR_ARG(str));
}

int main(void)
{
    dbg_init();

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

    for (int i = 4; i >= 0; i--)
    {
        hashtable_insert(&table, keys[i], names[i]);
        hashtable_print(&table, print_str);
    }
    hashtable_dump(&table);

    for (int i = 4; i >= 0; i--)
    {
        hashtable_delete(&table, keys[i]);
        hashtable_print(&table, print_str);
    }

    hashtable_print(&table, print_str);
    hashtable_dump(&table);

    hashtable_destroy(&table);

    dbg_destroy();
    return 0;
}


int oldmain(void)
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

    return 0;
}

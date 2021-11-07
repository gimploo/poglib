#include <stdio.h>
#include "../hashtable.h"


int main(void)
{
    str_t names[5] = {
        new_str("asldkfjas"),
        new_str("oqoewiruqweyr"),
        new_str(",zxmcn,mn"),
        new_str("9823weuiua"),
        new_str("][yopi.,m;lhj;hl]")
    };

    hashtable_t table = hashtable_init(100, str_t );

    hashtable_insert(&table, "jo", str_t, &names[0], str_t);
    hashtable_insert(&table, "jo", str_t, &names[2], str_t);
    hashtable_insert(&table, 32, u64, &names[1], str_t);


    return 0;
}


int oldmain(void)
{
    str_t name = new_str("gokul");
    u32 x = hash_cstr(name.buf, name.len);

    printf("string: "STR_FMT"\n", STR_ARG(&name));
    printf("index: %i\n", x % 10);

    str_t lol = new_str("gikul");
    u32 y = hash_cstr(lol.buf, lol.len);
    printf("string: "STR_FMT"\n", STR_ARG(&lol));
    printf("index: %i \n", y % 10);


    str_t juice = new_str("juice");
    u32 z = hash_cstr(juice.buf, lol.len);
    printf("string: "STR_FMT"\n", STR_ARG(&juice));
    printf("index: %i \n", z % 10);

    return 0;
}

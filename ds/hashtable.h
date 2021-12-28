#pragma once

#include "../basic.h"
#include "../str.h"



//NOTE: better to have hashtable take str_t type key instead of a number to avoid confusion
//NOTE: This hashtable allows to insert values either by reference or by value


typedef struct hashtable_t hashtable_t;

#define         hashtable_init(CAPACITY)                            __impl_hashtable_init(CAPACITY, DT_type(str_t))
#define         hashtable_insert(PTABLE, KEY, PVALUE, VALUE_TYPE)   __impl_hashtable_insert_key_value_pair_by_value(PTABLE,(const void *const)KEY, DT_type(str_t), (const void * const)PVALUE, DT_type(VALUE_TYPE)) 
#define         hashtable_delete(PTABLE, KEY)                       __impl_hashtable_delete_key_value_pair(PTABLE, (const void *)KEY, strlen((const char *)KEY), DT_type(str_t) )
#define         hashtable_destroy(PTABLE)                           __impl_hashtable_destroy(PTABLE)







#ifndef IGNORE_HASHTABLE_IMPLEMENTATION

#define HT_MAX_KEY_SIZE 64


typedef struct __key_value_pair_t {

    union {
        char      str[HT_MAX_KEY_SIZE];
        u64       ulong;
    } key;

    const void * const  value;
    const u32           __key_str_len;
    const data_type     key_type;
    const data_type     value_type;

} __key_value_pair_t;


struct hashtable_t {

    u64 len;
    __key_value_pair_t  *const __array;
    const u64           __capacity;
    const data_type     __value_type;
    bool                *const __index_table;
};


//NOTE: hashfunction from python3 docs
u64 hash_cstr(const char *word, const u64 word_len)
{
    u64 hash = 7;
    for (u64 i = 0; i < word_len; i++) {
        hash = hash*31 + word[i];
    }
    return hash;
}

u64 hash_u64(u64 x) 
{
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

u32 hash_u32(u32 x) 
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

u32 unhash_u32(u32 x) 
{
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
    return x;
}

u64 unhash_u64(u64 x) 
{
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return x;
}



hashtable_t __impl_hashtable_init(const u64 array_capacity, const data_type type)
{
    return (hashtable_t ) {
        .len = 0,
        .__array = (__key_value_pair_t *)calloc(array_capacity, sizeof(__key_value_pair_t)),
        .__capacity = array_capacity,
        .__value_type = type,
        .__index_table = (bool *)calloc(array_capacity, sizeof(bool)),
    };
}

bool __check_for_collision(const hashtable_t *table, const u64 index)
{
    return table->__index_table[index];
}

__key_value_pair_t __key_value_pair_init(const void *const key, const data_type key_type, const void * const value_addr, const data_type value_type)
{
    if (key_type != DT_str_t ) eprint("hashtable only supports str_t type keys");

    __key_value_pair_t output = {0};

    u32 str_len = strlen((char *)key);
    memcpy(output.key.str, (char *)key, HT_MAX_KEY_SIZE);

    return (__key_value_pair_t ) {
        .key = output.key,
        .value = value_addr,
        .__key_str_len = str_len,
        .key_type = key_type,
        .value_type = value_type,
    };
}

void __impl_hashtable_insert_key_value_pair_by_value(
        hashtable_t *table,
        const void *const key, 
        const data_type key_type, 
        const void * const value_addr, 
        const data_type value_type)
{ 
    const __key_value_pair_t kv = __key_value_pair_init(
            key,
            key_type,
            value_addr,
            value_type);

    if (table == NULL) eprint("table argument is null");
    if (value_type != table->__value_type) eprint("hashtable value type conflict");
    if (key_type != DT_str_t ) eprint("hashtable only supports str_t type keys");

    u64 index = hash_cstr(kv.key.str, kv.__key_str_len) % table->__capacity;


    if (!__check_for_collision(table, index)) {

        memcpy(table->__array + index, &kv, sizeof(__key_value_pair_t ));
        table->__index_table[index] = true;

    } else {

        eprint("hashtable %li index collision found", index);

    } 

    table->len++;
}

void __impl_hashtable_delete_key_value_pair(hashtable_t *table, const void * key_value, const u64 key_size, const data_type key_type)
{
    if (table == NULL) eprint("table argument is null");
    if (key_value == NULL) eprint("key argument is null");
    if (key_type != DT_str_t ) eprint("hashtable only supports str_t type key");

    u64 index_cstr = hash_cstr((const char *)key_value, key_size) % table->__capacity;
    table->__index_table[index_cstr] = false;

    table->len--;
}

void __impl_hashtable_destroy(hashtable_t *table)
{
    if (table == NULL) eprint("table arguemnt is null");

    free(table->__array);
    free(table->__index_table);
}

void hashtable_print(const hashtable_t *table, void (*print)(void*))
{
    assert(table);
    assert(print);

    printf("{\n");
    for (int i = 0; i < table->__capacity; i++)
    {
        if (table->__index_table[i]) {

            printf("\tkey: %s, value: ", table->__array[i].key.str);
            print((void *)table->__array[i].value);
            printf("\n");
        }
    }
    printf("}\n");

}

void hashtable_dump(const hashtable_t *table)
{
    assert(table);
    const char *output = 
        "=====================\n"
        "     INDEX | OCCUPIED \n"
        "=====================\n";

    printf("%s", output);
    for (int i = 0; i < table->__capacity; i++)
    {
        printf("       %02i | %s \n", i, table->__index_table[i] ? "TRUE" : "FALSE");
    }
    printf("=====================\n");
}
#endif

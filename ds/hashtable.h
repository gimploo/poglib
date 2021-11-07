#pragma once

#include "../basic.h"
#include "../str.h"

#define HT_MAX_KEY_SIZE 64



typedef struct hashtable_t hashtable_t;

#define hashtable_init(CAPACITY, TYPE) __impl_hashtable_init(CAPACITY, DT_type(TYPE))

//NOTE: This hashtable allows to insert values either by reference or by value
//
#define hashtable_insert(PTABLE, KEY, KEY_TYPE, PVALUE, VALUE_TYPE) do {\
\
    const __key_value_pair_t KV = __key_value_pair_init((const void *const)KEY, DT_type(KEY_TYPE), (const void * const)PVALUE, DT_type(VALUE_TYPE));\
    __impl_hashtable_insert_key_value_pair_by_value(PTABLE, &KV);\
\
} while(0)








#ifndef IGNORE_HASHTABLE_IMPLEMENTATION

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

typedef struct __key_value_pair_t __key_value_pair_t;

struct __key_value_pair_t {

    union {
        char      str[HT_MAX_KEY_SIZE];
        u64       ulong;
    } key;

    const void * const  value;
    const u32           __key_str_len;
    const data_type     key_type;
    const data_type     value_type;
};

struct hashtable_t {

    const u64           __array_size; 
    const u64           len;
    __key_value_pair_t  *const __array;
    data_type           __elem_type;
    bool                *const __index_table;
};


__key_value_pair_t __key_value_pair_init(const void *const key, const data_type key_type, const void * const value_addr, const data_type value_type)
{
    u32 str_len = 0;
    __key_value_pair_t output = {0};

    switch(key_type)
    {
        case DT_str_t:
            str_len = strlen((char *)key);
            memcpy(output.key.str, (char *)key, HT_MAX_KEY_SIZE);
            break;
        case DT_u64:
            output.key.ulong = (u64)key;
            break;
        default:
            eprint("type not accounted for");
    }

    return (__key_value_pair_t ) {
        .key = output.key,
        .value = value_addr,
        .__key_str_len = str_len,
        .key_type = key_type,
        .value_type = value_type,
    };
}


hashtable_t __impl_hashtable_init(const u64 array_capacity, const data_type type)
{
    return (hashtable_t ) {
        .__array_size = array_capacity * sizeof(__key_value_pair_t),
        .len = array_capacity,
        .__array = (__key_value_pair_t *)calloc(array_capacity, sizeof(__key_value_pair_t)),
        .__elem_type = type,
        .__index_table = (bool *)calloc(array_capacity, sizeof(bool)),
    };
}

bool __check_for_collision(const hashtable_t *table, const u64 index)
{
    return table->__index_table[index];
}

void __impl_hashtable_insert_key_value_pair_by_value(hashtable_t *table, const __key_value_pair_t *kv)
{
    if (table == NULL) eprint("table argument is null");
    if (kv == NULL) eprint("key value pair address is null");

    u64 index = 0;

    switch(kv->key_type)
    {
        case DT_str_t:

            index = hash_cstr(kv->key.str, kv->__key_str_len) % table->len;

        break;
        case DT_u64:

            index = hash_u64(kv->key.ulong) % table->len;

        break;
        default:
            eprint("hashtable: type not accounted for");
    }

    if (!__check_for_collision(table, index)) {
        memcpy(table->__array + index, kv, sizeof(__key_value_pair_t ));
        table->__index_table[index] = true;
    } else {
        eprint("hashtable %li index collision found", index);
    } 
}
#endif

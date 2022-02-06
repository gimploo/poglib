#pragma once
#define DEBUG

#include "../basic.h"
#include "../str.h"



//NOTE: better to have hashtable take str_t type key instead of a number to avoid confusion
//NOTE: This hashtable allows to insert values either by reference or by value


typedef struct hashtable_t hashtable_t;

#define         hashtable_init(CAPACITY, TYPE)                      __impl_hashtable_init(CAPACITY, #TYPE, sizeof(TYPE))
#define         hashtable_insert(PTABLE, KEY, VALUE)                __impl_hashtable_insert_key_value_pair_by_value(PTABLE, KEY, &VALUE, sizeof(VALUE))
#define         hashtable_delete(PTABLE, KEY)                       __impl_hashtable_delete_key_value_pair(PTABLE, KEY)
#define         hashtable_destroy(PTABLE)                           __impl_hashtable_destroy(PTABLE)

void            *hashtable_get_element_by_label(const hashtable_t *table, const char *label);




#ifndef IGNORE_HASHTABLE_IMPLEMENTATION

#define HT_MAX_KEY_SIZE 64


typedef struct __key_value_pair_t {

    const char      *key;
    void            *value;
    const u64       value_size;

} __key_value_pair_t;


struct hashtable_t {

    //private
    __key_value_pair_t  *__array;
    const u64           __capacity;
    bool                *__index_table;
    const char          *__elem_type;
    const u64           __elem_size;
    bool                __are_values_pointers;

    // public 
    u64 len;
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



hashtable_t __impl_hashtable_init(const u64 array_capacity, const char *elem_type, const u64 elem_size)
{
    bool flag = false;
    if (elem_type[strlen(elem_type) - 1] == '*') flag = true;

    hashtable_t o = {
        .__array = (__key_value_pair_t *)calloc(array_capacity, sizeof(__key_value_pair_t)),
        .__capacity = array_capacity,
        .__index_table = (bool *)calloc(array_capacity, sizeof(bool)),
        .__elem_type = elem_type,
        .__elem_size = elem_size,
        .__are_values_pointers = flag,
        .len = 0,
    };

    return o;
}

bool __check_for_collision(const hashtable_t *table, const u64 index)
{
    return table->__index_table[index];
}

__key_value_pair_t __key_value_pair_init(const char *key, void * value_addr, const u64 value_size)
{
    return (__key_value_pair_t ) {

        .key = key,
        .value = value_addr,
        .value_size = value_size

    };
}

void * __impl_hashtable_insert_key_value_pair_by_value(
        hashtable_t *table,
        const char  *key, 
        void        *value_addr, 
        const u64   value_size)
{ 
    if (table == NULL) eprint("table argument is null");
    if (value_size != table->__elem_size) eprint("expected value size (%li) but got (%li)", table->__elem_size, value_size);

    u64 index = hash_cstr(key, strlen(key)) % table->__capacity;
    if (!__check_for_collision(table, index)) {

        const __key_value_pair_t kv = __key_value_pair_init(key, value_addr, value_size);
        memcpy(table->__array + index, &kv, sizeof(__key_value_pair_t ));
        table->__index_table[index] = true;

    } else {

        eprint("hashtable %li index collision found", index);

    } 

    table->len++;
    
    return ((__key_value_pair_t *)table->__array + index)->value;
}

void __impl_hashtable_delete_key_value_pair(hashtable_t *table, const char *key_value)
{
    if (table == NULL) eprint("table argument is null");

    u64 index_cstr = hash_cstr(key_value, strlen(key_value)) % table->__capacity;
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
    for (u64 i = 0; i < table->__capacity; i++)
    {
        if (table->__index_table[i]) {

            printf("\tkey: %s, value: ", table->__array[i].key);
            print((void *)table->__array[i].value);
            printf("\n");
        }
    }
    printf("}\n");

}


void *hashtable_get_element_by_label(const hashtable_t *table, const char *label)
{
    assert(table);
    assert(label);

    u64 index = hash_cstr(label, strlen(label));

    if (!table->__index_table[index]) eprint("no value found at index %li of key %s\n", index, label);

    return table->__array[index].value;
}

void hashtable_dump(const hashtable_t *table)
{
    assert(table);
    const char *output = 
        "=====================\n"
        "     INDEX | OCCUPIED \n"
        "=====================\n";

    printf("%s", output);
    for (u64 i = 0; i < table->__capacity; i++)
    {
        printf("       %02li | %s \n", i, table->__index_table[i] ? "TRUE" : "FALSE");
    }
    printf("=====================\n");
}
#endif

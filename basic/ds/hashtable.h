#pragma once
#include "../common.h"
#include "./list.h"

//NOTE: better to have hashtable take str_t type key instead of a number to avoid confusion
//NOTE: This hashtable allows to insert values either by reference or by value

#define HT_MAX_KEY_LENGTH   63
#define HT_MAX_KEY_SIZE     HT_MAX_KEY_LENGTH + 1

typedef struct hashtable_t {

    // public 
    u64                 len;

    //private
    u8                  *__data;
    char                __elem_type[MAX_TYPE_CHARACTER_LENGTH];
    u64                 __elem_size;
    u64                 __capacity;
    bool                *__index_table;
    bool                __are_values_pointers;

} hashtable_t ;

#define         hashtable_init(CAPACITY, TYPE)                      __impl_hashtable_init((CAPACITY), (#TYPE), sizeof(TYPE))
#define         hashtable_insert(PTABLE, KEY, VALUE)                __impl_hashtable_insert_key_value_pair_by_value((PTABLE), (#KEY), &(VALUE), sizeof(VALUE))
#define         hashtable_delete(PTABLE, KEY)                       __impl_hashtable_delete_key_value_pair((PTABLE), (#KEY)) 

void *          hashtable_get_value(const hashtable_t *table, const char *key);

void            hashtable_print(const hashtable_t *table, void (*print)(void*));
void            hashtable_dump(const hashtable_t *table);

#define         hashtable_destroy(PTABLE)                           __impl_hashtable_destroy(PTABLE)


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



hashtable_t __impl_hashtable_init(const u64 array_capacity, const char *elem_type, const u64 elem_size)
{
    bool flag = false;
    u32 len = strlen(elem_type);
    if (elem_type[len] > MAX_TYPE_CHARACTER_LENGTH) eprint("variable name is too big, exceeded the %i limit threshold\n", MAX_TYPE_CHARACTER_LENGTH);
    if (elem_type[len - 1] == '*') flag = true;

    hashtable_t o = {
        .len = 0,
        .__data               = (u8 *)calloc(array_capacity, elem_size),
        .__elem_type           = {0},
        .__elem_size           = elem_size,
        .__capacity             = array_capacity,
        .__index_table          = (bool *)calloc(array_capacity, sizeof(bool)),
        .__are_values_pointers  = flag,
    };

    if (!flag)  memcpy(o.__elem_type, elem_type, MAX_TYPE_CHARACTER_LENGTH);
    else        memcpy(o.__elem_type, elem_type, len - 1);

    return o;
}

bool __check_for_collision(const hashtable_t *table, const u64 index)
{
    return table->__index_table[index];
}

void * __hashtable_get_reference_to_only_value_at_index(const hashtable_t *table, const u64 index)
{
    if (!table->__are_values_pointers)
        return (void *)(table->__data + index * table->__elem_size);
    else 
        return *(void **)(table->__data + index * table->__elem_size);
}


void * __impl_hashtable_insert_key_value_pair_by_value(
        hashtable_t *table,
        const char  *key, 
        const void  *value_addr, 
        const u64   value_size)
{ 
    assert(table);
    if (strlen(key) > HT_MAX_KEY_LENGTH) eprint("%s key cant be greater than %i character long\n", key, HT_MAX_KEY_SIZE);
    if (value_size != table->__elem_size) eprint("expected value size (%li) but got (%li)", table->__elem_size, value_size);

    u64 index = hash_cstr(key, strlen(key)) % table->__capacity;
    assert(index >= 0 && index < table->__capacity);

    if (!__check_for_collision(table, index)) {

        memcpy(
            table->__data + (index * table->__elem_size), 
            value_addr, 
            table->__elem_size);

        table->__index_table[index] = true;

    } else {

        eprint("hashtable %li index collision found", index);

    } 

    table->len++;
    
    return __hashtable_get_reference_to_only_value_at_index(table, index);
}

void __impl_hashtable_delete_key_value_pair(hashtable_t *table, const char *key_value)
{
    if (table == NULL) eprint("table argument is null");

    u64 index_cstr = hash_cstr(key_value, strlen(key_value)) % table->__capacity;
    assert(index_cstr >= 0 && index_cstr < table->__capacity);

    table->__index_table[index_cstr] = false;

    table->len--;
}


void __impl_hashtable_destroy(hashtable_t *table)
{
    if (table == NULL) eprint("table arguemnt is null");

    free(table->__data);
    free(table->__index_table);
}

void hashtable_print(const hashtable_t *table, void (*print)(void*))
{
    assert(table);
    assert(print);

    printf("{\n");

    for (u64 i = 0; i < table->__capacity; i++)
    {
        if (!table->__index_table[i]) continue;
        print(__hashtable_get_reference_to_only_value_at_index(table, i));
        printf(",\n");
    }
    printf("}\n");

}


void * hashtable_get_value(const hashtable_t *table, const char *key)
{
    assert(table);
    assert(key);

    const u64 index = hash_cstr(key, strlen(key)) % table->__capacity;

    if (!table->__index_table[index]) eprint("`%s` key doesnt have an entry in table", key);

    return __hashtable_get_reference_to_only_value_at_index(table, index);
}

void hashtable_dump(const hashtable_t *table)
{
    assert(table);

    printf("len = %li\ncapacity = %li\nelem_size = %li\nelem_type = %s\nare_values_are_pointers = %s\n",
         table->len,
         table->__capacity,
         table->__elem_size,
         table->__elem_type,
         table->__are_values_pointers ? "TRUE" : "FALSE");

    const char *output = 
        "=====================\n"
        "   INDEX | OCCUPIED \n"
        "=====================\n";

    printf("%s", output);
    for (u64 i = 0; i < table->__capacity; i++)
    {
        printf("%02li | %s \n", i, table->__index_table[i] ? "TRUE" : "FALSE");
    }
    printf("=====================\n");
}
#endif

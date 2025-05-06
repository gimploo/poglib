#pragma once
#include "../common.h"
#include "./list.h"
#include "../str.h"
#include "./slot.h"
#include "../util.h"

typedef struct table_entry_t {
    str_t   key;
    void    *value;
    u32     value_size;
    u32     probe_distance;
    bool    is_occupied;
} table_entry_t;

typedef struct hashtable_t {
    slot_t entries;
} hashtable_t ;

void hashtable_insert(hashtable_t *table, const char *key, void *value, const u32 value_size);

#ifndef IGNORE_HASHTABLE_IMPLEMENTATION

//NOTE: hash function apparently is really good for strings keys -- //fnv1a_hash
uint32_t hash_cstr(const char *key) {
    uint32_t hash = 2166136261u;
    while (*key) {
        hash ^= (unsigned char)*key++;
        hash *= 16777619u;
    }
    return hash;
}

void hashtable_insert(hashtable_t *table, const char *key, void *value, const u32 value_size)
{
    u32 index = hash_cstr(key);
    u32 probe_distance = 0;

    while(true) {
        table_entry_t *entry = slot_get_value(&table->entries,index);

        if(!entry->is_occupied) {
            slot_insert(&table->entries, index, entry, sizeof(table_entry_t));
            entry->probe_distance = probe_distance;
            entry->is_occupied = true;
            return;
        }

        if(!strcmp(entry->key.data, key)) {
            entry->value = value;
            return;
        }

        if(probe_distance > entry->probe_distance){
            swap_memory((char *)key, entry->key.data, sizeof(str_t));
            swap_memory(value, entry->value, entry->value_size);
            swap_memory(&probe_distance, &entry->probe_distance, sizeof(probe_distance));
        }

        // Move to next slot
        index = (index + 1) % slot_get_capacity(&table->entries);
        probe_distance += 1;
    }
}

void *hashtable_get_value(const hashtable_t *table, const char *key)
{
    u32 index = hash_cstr(key);
    u32 probe_distance = 0;

    while(true){
        table_entry_t *entry = slot_get_value(&table->entries,index);

        if(!entry->is_occupied)
            return NULL;

        if(!strcmp(entry->key.data, key)){
            return entry->value;
        }

        if(probe_distance > entry->probe_distance) {
            return NULL;
        }

        index = (index + 1) % slot_get_capacity(&table->entries);
        probe_distance += 1;
    }
}


#endif

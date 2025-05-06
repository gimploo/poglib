#pragma once
#include "../common.h"
#include "./list.h"
#include "../str.h" 
#include "./slot.h"
#include "../util.h"

//NOTE: hashtable inlines data that are of 8 bytes less and pointers of 8 bytes more

typedef struct table_entry_t {
    str_t   key;
    union {
        void    *ptr;
        u8      data[sizeof(void *)];
    };
    u32     probe_distance;
    bool    is_occupied;
} table_entry_t;

typedef struct hashtable_t {
    slot_t entries;
    enum value_mode_t {
        VALUE_MODE_POINTER = 0,
        VALUE_MODE_INLINE_COPY = 1,
        VALUE_MODE_COUNT
    } mode;
    const u32 type_size;
} hashtable_t ;

#define hashtable_init(CAPACITY, TYPE)\
    __impl_hashtable_init((CAPACITY), sizeof(TYPE))

#define hashtable_insert(TABLE, KEY, VALUE) \
    _Generic((VALUE), \
        int:         __hashtable_insert_val, \
        unsigned:    __hashtable_insert_val, \
        long:        __hashtable_insert_val, \
        float:       __hashtable_insert_val, \
        double:      __hashtable_insert_val, \
        char:        __hashtable_insert_val, \
        short:       __hashtable_insert_val, \
        long long:   __hashtable_insert_val, \
        default:     __hashtable_insert_ptr \
    )((TABLE), (KEY), (VALUE))

void            hashtable_delete(hashtable_t *table, const char *key);
const void *    hashtable_get_value(const hashtable_t *table, const char *key);
void            hashtable_print(const hashtable_t *table, void (*print)(void *));
void            hashtable_destroy(hashtable_t *table);

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


hashtable_t __impl_hashtable_init(const u32 capacity, const u32 value_size)
{
    ASSERT(capacity > 0);
    ASSERT(value_size > 0);

    return (hashtable_t ) {
        .entries = slot_init(capacity, table_entry_t),
        .mode = value_size >= sizeof(void *) ? VALUE_MODE_POINTER : VALUE_MODE_INLINE_COPY,
        .type_size = value_size
    };
}

table_entry_t * hashtable_insert_raw(hashtable_t *table, const char *key, void *value)
{
    if (table->entries.len == slot_get_capacity(&table->entries)) {
        eprint("Exceeded limit");
    }

    const u32 entries_capacity = slot_get_capacity(&table->entries);
    u32 index = hash_cstr(key) % entries_capacity;
    u32 probe_distance = 0;
    str_t str_key = str_init(key);

    while(true) {
        table_entry_t *entry = slot_get_value(&table->entries,index);

        if(!entry->is_occupied) {
            printf("Inserting %s at index %u, probe_distance %u\n", key, index, probe_distance);
            return slot_insert(
                &table->entries, 
                index, 
                &(table_entry_t) {
                    .key = str_key,
                    .ptr = value,
                    .probe_distance = probe_distance,
                    .is_occupied = true,
                }, 
                sizeof(table_entry_t)
            );
        }

        if(!strcmp(entry->key.data, key)) {
            str_free(&str_key);
            entry->ptr = value;
            return entry;
        }

        if(probe_distance > entry->probe_distance){
            swap_memory(&str_key, &entry->key, sizeof(str_t));
            if (table->mode == VALUE_MODE_POINTER)
                swap(value, entry->ptr);
            else
                swap_memory(value, entry->data, table->type_size);
            swap_memory(&probe_distance, &entry->probe_distance, sizeof(probe_distance));
        }

        // Move to next slot
        index = (index + 1) % entries_capacity;
        probe_distance += 1;
    }
}

// For small data types (≤ 8 bytes) passed by value
static inline void * __hashtable_insert_val(hashtable_t *table, const char *key, int val)
{
    ASSERT(table->mode == VALUE_MODE_INLINE_COPY);
    return hashtable_insert_raw(table, key, &val)->data;
}

// For actual pointers (≥ 8 bytes or heap data)
static inline void * __hashtable_insert_ptr(hashtable_t *table, const char *key, void *ptr)
{
    ASSERT(table->mode == VALUE_MODE_POINTER);
    return hashtable_insert_raw(table, key, ptr)->ptr;
}

const void * __get_value(const hashtable_t *table, const table_entry_t *entry) 
{
    return table->mode == VALUE_MODE_POINTER ? entry->ptr : entry->data;
}

const void * hashtable_get_value(const hashtable_t *table, const char *key)
{
    const u32 entries_capacity = slot_get_capacity(&table->entries);
    u32 index = hash_cstr(key) % entries_capacity;
    u32 probe_distance = 0;

    while(true){
        const table_entry_t *entry = slot_get_value(&table->entries,index);

        if(!entry->is_occupied)
            eprint("No entry");

        if(!strcmp(entry->key.data, key)){
            return __get_value(table, entry);
        }

        if(probe_distance > entry->probe_distance) {
            eprint("Error here");
        }

        index = (index + 1) % entries_capacity;
        probe_distance += 1;
    }
}


void hashtable_delete(hashtable_t *table, const char *key)
{
    const u32 entries_capacity = slot_get_capacity(&table->entries);
    u32 index = hash_cstr(key) % entries_capacity;
    u32 probe_distance = 0;

    while(true) {
        table_entry_t *entry = slot_get_value(&table->entries, index);

        if(!entry->is_occupied) {
            eprint("Tried to access an unoccupied element - investigate upstream");
        }

        if(!strcmp(entry->key.data, key)){
            break;
        }

        if(probe_distance > entry->probe_distance){
            eprint("Tried to probe further - investigate upstream");
        }

        index = (index + 1) % entries_capacity;
        probe_distance += 1;
    }

    //Remove entry
    u32 current = index;
    slot_delete(&table->entries, current);

    //Backward shift entries to fill the gap
    u32 next = (current + 1) % entries_capacity;

    while(true) {
        table_entry_t *next_entry = slot_get_value(&table->entries, next);
        if (!next_entry->is_occupied || next_entry->probe_distance == 0) 
            break;

        // Move next entry back to current position
        table_entry_t *current_entry = slot_get_value(&table->entries, current);
        *current_entry = *next_entry;
        current_entry->probe_distance -= 1;

        // Clear next position
        next_entry->is_occupied = false;

        // Advance pointers
        current = next;
        next = (next + 1) % entries_capacity;
    }

}

void hashtable_destroy(hashtable_t *table)
{
    slot_iterator(&table->entries, iter)
    {
        table_entry_t *entry = iter;
        str_free(&entry->key);
    }
    slot_destroy(&table->entries);
}

void hashtable_print(const hashtable_t *table, void (*print)(void *)) {
    if (table == NULL) {
        printf("Hashtable is NULL\n");
        return;
    }

    // Print basic hashtable information
    printf("Hashtable Details:\n");
    printf("  Type Size: %u\n", table->type_size);
    printf("  Value Mode: %s\n", table->mode ? "VALUE" : "POINTER");
    printf("  Entries Capacity: %u\n", slot_get_capacity(&table->entries));
    printf("  Number of Entries (Occupied): ");

    u32 occupied_count = 0;
    slot_iterator(&table->entries, iter) {
        table_entry_t *entry = iter;
        if (entry->is_occupied) {
            occupied_count++;
        }
    }
    printf("%u\n", occupied_count);

    // Print the entries in the hashtable
    printf("  Entries:\n");
    slot_iterator(&table->entries, iter) {
        table_entry_t *entry = iter;
        if (entry->is_occupied) {
            printf("    Key: %s\n", entry->key.data);
            printf("    Probe Distance: %u\n", entry->probe_distance);
            printf("    Value: ");
            if (table->mode == VALUE_MODE_POINTER) {
                printf("Pointer: %p\n", entry->ptr);
                printf("    Data: ");
                print(entry->ptr);
            } else if (table->mode == VALUE_MODE_INLINE_COPY) {
                // Print value inline if it's small data
                for (u32 i = 0; i < table->type_size; ++i) {
                    printf("%02x ", entry->data[i]);
                }
                printf("\n");
            }
        }
    }
}

#define hashtable_get_entry_value(ENTRY) ((table_entry_t *)ENTRY)->ptr

#define hashtable_iterator(TABLE, ENTRY) slot_iterator(&(TABLE)->entries, (ENTRY))


#endif

#pragma once
#include "../common.h"
#include "../str.h" 
#include "./slot.h"
#include "../util.h"
#include "../arena.h"
#include "../runtime-ctx.h"

//NOTE: Things to know before using 
//-------------------------------------
//1. Hashtable will only hold pointer to allocated pointers and doesnt not keep a 
//   copy of the pointer value internally
//2. Hashtable inlines data that are of 8 bytes less and pointers of 8 bytes more
//3. Uses round robin collision resolution

typedef enum ht_key_type {
    HT_KEY_TYPE_STR = 0, 
    HT_KEY_TYPE_U32 = 1,
    HT_KEY_TYPE_COUNT,
} ht_key_type;

typedef struct hashtable_entry_t hashtable_entry_t;

typedef union hashtable_key_t hashtable_key_t;
union hashtable_key_t {
    str_t str;
    u32 u32; 
};

typedef struct hashtable_t {
    slot_t keys;
    slot_t entries;
    struct {
        ht_key_type keytype;
        bool value_stored_as_pointers;
        arena_t keypool;
    } internal;
} hashtable_t ;


#define         hashtable_init(CAPACITY, KEY_TYPE, VALUE_TYPE, PARENA)\
                hashtable__internal_init((CAPACITY), KEY_TYPE, sizeof(VALUE_TYPE), (PARENA))

#define         hashtable_insert(PTABLE, KEY, VALUE)\
                hashtable__internal_insert((PTABLE), (KEY), (void *)(u64)(VALUE))

#define         hashtable_iterator(TABLE, ENTRY)\
                slot_iterator(&(TABLE)->entries, (ENTRY))

void            hashtable_delete(hashtable_t * const table, const hashtable_key_t key);
const void *    hashtable_get_value(const hashtable_t * const table, const hashtable_key_t key);
void            hashtable_print(const hashtable_t * const table, void (*print)(void *));
bool            hashtable_has_key(const hashtable_t * const table, const hashtable_key_t key);
hashtable_key_t hashtable_get_key_from_value(hashtable_t * const self, const void * const value);
void            hashtable_destroy(hashtable_t * const table);


#ifndef IGNORE_HASHTABLE_IMPLEMENTATION

struct hashtable_entry_t {
    hashtable_key_t key;
    void    *value;
    u32     probe_distance;
    bool    is_occupied;
};


//NOTE: hash function apparently is really good for strings keys -- //fnv1a_hash
u32 ht__internal__hash_str(const str_t key) {
    u32 hash = 2166136261u;
    for(u32 i = 0; i < key.len; i++) 
    {
        hash ^= (u8)key.data[i];
        hash *= 16777619u;
    }
    return hash;
}

//NOTE: Knuth's multiplicative hash for 32-bit integers
u32 ht__internal_hash_u32(const u32 key) {
    return key * 2654435761u;
}

hashtable_t hashtable__internal_init(const u32 capacity, const ht_key_type keytype, const u32 value_size, arena_t * const arena)
{
    ASSERT(capacity > 0);
    ASSERT(value_size > 0);
    ASSERT(keytype < HT_KEY_TYPE_COUNT && keytype >= HT_KEY_TYPE_STR);
    if (!global_runtimectx) {
        eprint("Requries runtimectx to ensure better cache locality");
    }

    return (hashtable_t ) {
        .entries = slot_init(capacity, sizeof(hashtable_entry_t), false, arena),
        .internal = {
            .value_stored_as_pointers = value_size >= sizeof(void *),
            .keytype = keytype,
            .keypool = runtimectx_reserve_mem_from_stringpool(250),
        }
    };
}

bool hashtable__internal_compare_key(const hashtable_t * const self, const hashtable_key_t key1, const hashtable_key_t key2) {
    return self->internal.keytype == HT_KEY_TYPE_STR 
        ? key1.str.len == key2.str.len && (strncmp(key1.str.data, key2.str.data, key1.str.len) == 0)
        : key1.u32 == key2.u32;
}

u32 hashtable__internal_get_hashed_key_index(const hashtable_t * const self, const hashtable_key_t key)
{
    const ht_key_type keytype = self->internal.keytype;
    if (keytype == HT_KEY_TYPE_STR) ASSERT(key.str.len > 0);

    return keytype == HT_KEY_TYPE_STR 
        ? ht__internal__hash_str(key.str) % slot_get_capacity(&self->entries) 
        : ht__internal_hash_u32(key.u32) % slot_get_capacity(&self->entries);
}

void hashtable__internal_insert(hashtable_t * const table, const hashtable_key_t key, void *value)
{
    if (table->entries.len == slot_get_capacity(&table->entries)) {
        eprint("Exceeded limit");
    }

    const u32 entries_capacity = slot_get_capacity(&table->entries);

    u32 index = hashtable__internal_get_hashed_key_index(table, key);
    u32 probe_distance = 0;
    hashtable_key_t swap_key = key;
    while(true) {

        hashtable_entry_t * const entry = table->entries.len
            ? slot_get_value(&table->entries,index) 
            : NULL;

        if(!entry || !entry->is_occupied) {
            hashtable_entry_t newentry = {
                .key = swap_key,
                .value = value,
                .probe_distance = probe_distance,
                .is_occupied = true,
            };
            slot_insert(
                &table->entries, 
                index, 
                &newentry,
                sizeof(hashtable_entry_t)
            );
            return;
        }

        //NOTE: override value of an existing hashtable entry
        if (hashtable__internal_compare_key(table, swap_key, entry->key)) {
            entry->value = value;
            return;
        }

        if(probe_distance > entry->probe_distance){
            swap((void *)&value, &entry->value);
            swap_memory(&swap_key, &entry->key, sizeof(hashtable_key_t));
            swap_memory(&probe_distance, &entry->probe_distance, sizeof(probe_distance));
        }

        // Move to next slot
        index = (index + 1) % entries_capacity;
        probe_distance += 1;
    }
}

const void * hashtable_get_value(const hashtable_t *table, const hashtable_key_t key)
{
    const u32 entries_capacity = slot_get_capacity(&table->entries);
    u32 index = hashtable__internal_get_hashed_key_index(table, key);

    u32 probe_distance = 0;

    while(true){
        const hashtable_entry_t *entry = slot_get_value(&table->entries,index);

        if(!entry->is_occupied)
            eprint("No entry");

        if (hashtable__internal_compare_key(table, key, entry->key)) {
            return entry->value;
        }

        if(probe_distance > entry->probe_distance) {
            eprint("Error here");
        }

        index = (index + 1) % entries_capacity;
        probe_distance += 1;
    }
}


void hashtable_delete(hashtable_t * const table, const hashtable_key_t key)
{
    const u32 entries_capacity = slot_get_capacity(&table->entries);
    u32 index = hashtable__internal_get_hashed_key_index(table, key);
    u32 probe_distance = 0;

    while(true) {
        hashtable_entry_t *entry = slot_get_value(&table->entries, index);

        if(!entry->is_occupied) {
            eprint("Tried to access an unoccupied element - investigate upstream");
        }

        if (hashtable__internal_compare_key(table, key, entry->key)) {
            slot_delete(&table->entries, index);
            break;
        }

        if(probe_distance > entry->probe_distance){
            eprint("Tried to probe further - investigate upstream");
        }

        index = (index + 1) % entries_capacity;
        probe_distance += 1;
    }

    u32 current = index;
    u32 next = (current + 1) % entries_capacity;
    while(true) {
        hashtable_entry_t *next_entry = slot_get_value(&table->entries, next);
        if (!next_entry->is_occupied || next_entry->probe_distance == 0) 
            break;
        next_entry->probe_distance -= 1;

        // Move next entry back to current position
        slot_insert(&table->entries, current, next_entry, sizeof(hashtable_entry_t));
        slot_delete(&table->entries, next);

        // Advance pointers
        current = next;
        next = (next + 1) % entries_capacity;
    }

}

void hashtable_destroy(hashtable_t *table)
{
    slot_destroy(&table->entries);
    arena_destroy(&table->internal.keypool);
}

bool hashtable_has_key(const hashtable_t *table, const hashtable_key_t key)
{
    const u32 entries_capacity = slot_get_capacity(&table->entries);
    u32 index = hashtable__internal_get_hashed_key_index(table, key);
    u32 probe_distance = 0;

    if (!slot_is_index_occupied(&table->entries, index))
        return false;

    while(true){
        const hashtable_entry_t *entry = slot_get_value(&table->entries, index);

        if(!entry->is_occupied || probe_distance > entry->probe_distance) {
            return false;
        }

        if (hashtable__internal_compare_key(table, key, entry->key)) {
            return true;
        }

        index = (index + 1) % entries_capacity;
        probe_distance += 1;
    }
}

hashtable_key_t hashtable_get_key_from_value(hashtable_t * const self, const void * const value)
{
    ASSERT(value);
    hashtable_iterator(self, iter)
    {
        const hashtable_entry_t *entry = iter;
        if (entry->value == value) {
            return entry->key;
        }
    }
    eprint("tried to get key from a value that doesnot exist in the table");
}

#endif

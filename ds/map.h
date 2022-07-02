#pragma once
#include "./list.h"
#include "./hashtable.h"


/*=============================================================================
                            - MAP DATA STRUCTURE -
=============================================================================*/

typedef struct map_t {

    u64         len;
    list_t      __keys;
    hashtable_t __values;

} map_t ;



#define         map_init(CAPACITY, TYPE)                __impl_map_init(CAPACITY, #TYPE, sizeof(TYPE))
#define         map_insert(PMAP, KEY, VALUE)            __impl_map_insert(PMAP, (KEY), &(VALUE), sizeof(VALUE))
#define         map_delete(PMAP, KEY)                   __impl_map_delete((PMAP), (KEY))

void *          map_get_value(const map_t *map, const char *key);
#define         map_get_value_at_index(PMAP, INDEX)     __impl_map_get_reference_to_value_at_index(PMAP, (INDEX))

#define         map_iterator(PMAP, TMP)                 __impl_map_for_loop_iterator(PMAP, TMP)

void            map_destroy(map_t *);



/*-----------------------------------------------------------------------------
                                IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_MAP_IMPLEMENTATION


void * map_get_value(const map_t *map, const char *key)
{
    assert(map);
    assert(key);

    const list_t *keys = &map->__keys;
    list_iterator(keys, iter) {
        printf("%s == %s\n", key, (const char *)iter);
        if (strcmp(key, (const char *)iter) == 0) {
            const hashtable_t *table = &map->__values;
            return hashtable_get_value(table, key);
        } 
    }

    eprint("`%s` key not found in map\n", key);
}

#define __impl_map_for_loop_iterator(PMAP, TMP)\
                if ((PMAP)->len != 0)\
                    for (void *index = 0, *(TMP) = (void *)map_get_value((PMAP), (char *)list_get_value(&(PMAP)->__keys, 0));\
                             (u64)(index) < (PMAP)->len;\
                             index = (void *)((u64)index + 1),\
                             (TMP) = (void *)map_get_value((PMAP), \
                                 (char *)list_get_value(&(PMAP)->__keys, \
                                     ((u64)index < (PMAP)->len) ? \
                                         (u64)index : \
                                         (u64)((u64)index - 1))))

map_t __impl_map_init(const u64 capacity, const char *type_name, const u32 elem_size)
{
    map_t o = {
        .len = 0,
        .__keys = list_init(capacity, char[HT_MAX_KEY_SIZE]),
        .__values = __impl_hashtable_init(capacity, type_name, elem_size),
    };

    return o;
}

void * __impl_map_insert(map_t *self, const char *key, const void *value_addr, const u64 value_size)
{
    assert(self);
    char buf[HT_MAX_KEY_SIZE] = {0};
    memcpy(buf, key, HT_MAX_KEY_SIZE);

    list_append(&self->__keys, buf);
    self->len++;
    return __impl_hashtable_insert_key_value_pair_by_value(&self->__values, key, value_addr, value_size);
}


void __impl_map_delete(map_t *self, const char *key)
{
    hashtable_delete(&self->__values, key);
    for (u32 i = 0; i < self->__keys.len; i++)
    {
        const char *tmp = (char *)list_get_value(&self->__keys, i);
        if (strcmp(key, tmp) == 0)
        {
            list_delete(&self->__keys, i);
            self->len--;
            return;
        }
    }
    eprint("key `%s` not found", key);
}

void *__impl_map_get_reference_to_value_at_index(const map_t *map, const u32 index)
{
    if (index >= map->__keys.len) 
        eprint("Index value exceeds map.__keys length");

    const list_t *keys  = &map->__keys;
    const char *key     = (char *)list_get_value(keys, index);
    const u64 hash      = hash_cstr(key, strlen(key)) % map->__values.__capacity;

    return __hashtable_get_reference_to_only_value_at_index(&map->__values, hash);
}

void map_destroy(map_t *map)
{
    assert(map);
    list_destroy(&map->__keys);
    hashtable_destroy(&map->__values);
    map->len = 0;
}
#endif

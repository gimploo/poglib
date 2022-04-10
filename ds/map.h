#pragma once
#include "./list.h"
#include "./hashtable.h"


/*=============================================================================
                            - MAP DATA STRUCTURE -
=============================================================================*/


typedef struct map_t map_t ;

#define         map_init(CAPACITY, TYPE)                __impl_map_init(CAPACITY, #TYPE, sizeof(TYPE))
#define         map_insert(PMAP, KEY, VALUE)            __impl_map_insert(PMAP, (KEY), &(VALUE), sizeof(VALUE))
#define         map_delete(PMAP, KEY)                   __impl_map_delete((PMAP), (KEY))

#define         map_get_value(PMAP, KEY)                hashtable_get_value(&(PMAP)->__values, KEY) 
#define         map_get_value_at_index(PMAP, INDEX)     __impl_map_get_reference_to_value_at_index(PMAP, INDEX)

#define         map_iterator(PMAP, TMP)                 __impl_map_for_loop_iterator(PMAP, TMP)

void            map_destroy(map_t *);


#ifndef IGNORE_MAP_IMPLEMENTATION


struct map_t {

    u64         len;
    list_t      __keys;
    hashtable_t __values;

};

#define __impl_map_for_loop_iterator(PMAP, TMP)\
                    u64 TMP##_I = 0;\
                    for (void *(TMP) = (void *)map_get_value((PMAP), (char *)list_get_value(&(PMAP)->__keys, 0));\
                         (TMP##_I) < (PMAP)->len;\
                         (TMP) = map_get_value((PMAP), \
                             (char *)list_get_value(&(PMAP)->__keys, \
                                 (++TMP##_I < (PMAP)->len) ? TMP##_I : (TMP##_I - 1))))

map_t __impl_map_init(const u64 capacity, const char *type_name, const u32 elem_size)
{
    map_t o = {
        .len = 0,
        .__keys = list_init(capacity, char * ),
        .__values = __impl_hashtable_init(capacity, type_name, elem_size),
    };

    return o;
}

void * __impl_map_insert(map_t *self, const char *key, const void *value_addr, const u64 value_size)
{
    assert(self);

    list_append(&self->__keys, key);
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

void *__impl_map_get_reference_to_value_at_index(map_t *map, const u32 index)
{
    if (index > map->__keys.len) eprint("Index value exceeds map.__kyes length");
    return __hashtable_get_reference_to_only_value_at_index(&map->__values, index);
}

void map_destroy(map_t *map)
{
    assert(map);
    list_destroy(&map->__keys);
    hashtable_destroy(&map->__values);
    map->len = 0;
}
#endif

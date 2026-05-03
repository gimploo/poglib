#pragma once
#include "../common.h"
#include "../arena.h"

/*============================================================================
                - STATIC ARRAY (SLOT ARRAY ) DATA STRUCTURE -
============================================================================*/

typedef struct slot_t {

    u8 *data;
    u32 len;

    struct {
        bool    stored_as_pointers;
        bool    *index_table;
        u64     elem_size;
        u64     capacity;
        arena_t *arena;
    } internal;

} slot_t ;


slot_t              slot_init(const u64 array_capacity, const u64 elem_size, const bool store_as_pointers, arena_t * const arena);
void *              slot_insert(slot_t *, const u64 index, const void *value, const u64 value_size);
void *              slot_update(slot_t *, const u64 index, const void *value, const u64 value_size);
void                slot_insert_multiple(slot_t *self, const u8 *arraybuffer, const u32 arraylen, const u32 elem_size);
bool                slot_is_index_occupied(const slot_t * const self, const u32 index);
#define             slot_append(PSLOTARRAY, VALUE)                         slot_insert((PSLOTARRAY), (PSLOTARRAY)->len, &(VALUE), sizeof(VALUE))
#define             slot_delete(PSLOTARRAY, INDEX)                         __impl_slot_delete((PSLOTARRAY), (INDEX))
slot_t              slot_clone(const slot_t *slot);
void *              slot_get_value(const slot_t *table, const u64 index);
#define             slot_iterator(PSLOTARRAY, ITER)                        __impl_slot_for_loop_iterator((PSLOTARRAY), (ITER))
#define             slot_get_capacity(PSLOT)                                (PSLOT)->internal.capacity
#define             slot_get_buffer(PSLOT)                                  (PSLOT)->data
#define             slot_get_size(PSLOT)                                    ((PSLOT)->internal.elem_size * (PSLOT)->len)
#define             slot_get_elem_size(PSLOT)                               ((PSLOT)->internal.elem_size)
void                slot_print(const slot_t *table, void (*print)(void*));
void                slot_dump(const slot_t *table);
void                slot_clear(slot_t *);

#define             slot_destroy(PSLOTARRAY)                               __impl_slot_destroy(PSLOTARRAY)



#ifndef IGNORE_SLOTARRAY_IMPLEMENTATION

/*------------------------------------------------------------------------------
                             IMPLEMENTATION 
------------------------------------------------------------------------------*/


void * __slot_iter_get_value(const slot_t *slot, u32 *hits, u32 *index)
{
    while(*index < slot->internal.capacity && *hits < slot->len) {

        if (slot->internal.index_table[*index]) {
            *hits = *hits + 1;
            return slot_get_value(slot, *index);
        }

        *index = *index + 1;
    }

    return NULL;
}

#define __impl_slot_for_loop_iterator(PSLOTARRAY, ITER)\
            if ((PSLOTARRAY)->len != 0)\
                for (void **SLT__index = 0, **SLT__hits = 0, *(ITER) = (void *)__slot_iter_get_value((PSLOTARRAY),(u32 *)&SLT__hits, (u32 *)&SLT__index);\
                    (ITER) != NULL;\
                    SLT__index = (void **)((u64)SLT__index + 1),\
                        (ITER) = (void *)__slot_iter_get_value(PSLOTARRAY, (u32 *)&SLT__hits, (u32 *)&SLT__index))



slot_t slot_init(const u64 array_capacity, const u64 elem_size, const bool store_as_pointers, arena_t * const arena)
{
    ASSERT(elem_size > 0);

    slot_t o = {
      .len = 0,
      .data                     = arena ? arena_reserve(arena, array_capacity * elem_size) : (u8 *)calloc(array_capacity, elem_size),
      .internal = {
          .elem_size            = elem_size,
          .capacity             = array_capacity,
          .index_table          = arena ? arena_reserve(arena, array_capacity) : (u8 *)calloc(array_capacity, sizeof(bool)),
          .stored_as_pointers   = store_as_pointers,
          .arena                = arena,
      }
    };

    return o;
}

bool slot_is_index_occupied(const slot_t * const table, const u32 index)
{
    return table->internal.index_table[index];
}

void * __slot_get_reference_to_only_value_at_index(const slot_t *table, const u64 index)
{
    if (!table->internal.stored_as_pointers)
        return (void *)(table->data + index * table->internal.elem_size);
    else 
        return *(void **)(table->data + index * table->internal.elem_size);
}


void * slot_insert(
        slot_t * const table,
        const u64 index, 
        const void *value_addr, 
        const u64 value_size)
{ 
    if (table == NULL) eprint("table argument is null");
    if (value_size != table->internal.elem_size) eprint("expected value size (%li) but got (%li)", table->internal.elem_size, value_size);
    ASSERT(index >= 0 && index < table->internal.capacity);

    if (slot_is_index_occupied(table, index)) {
        eprint("slot at [%li] index is not empty", index);
    }

    if (value_addr) {
        memcpy(table->data + (index * table->internal.elem_size), value_addr, table->internal.elem_size);
    } else {
        memset(table->data + (index * table->internal.elem_size), 0, table->internal.elem_size);
    }

    table->internal.index_table[index] = true;
    table->len++;
    return __slot_get_reference_to_only_value_at_index(table, index);
}

void * slot_update(
        slot_t *table,
        const u64   index, 
        const void  *value_addr, 
        const u64   value_size)
{
    if (table == NULL) eprint("table argument is null");
    if (value_size != table->internal.elem_size) eprint("expected value size (%li) but got (%li)", table->internal.elem_size, value_size);
    ASSERT(index >= 0 && index < table->internal.capacity);

    if (slot_is_index_occupied(table, index)) {

        memcpy(
            table->data + (index * table->internal.elem_size), 
            value_addr, 
            table->internal.elem_size);

        table->internal.index_table[index] = true;

    } else {

        logging("slot at [%li] index is empty - use slot_insert instead (better for readability)", index);
        slot_insert(table, index, value_addr, value_size);

    } 

    return __slot_get_reference_to_only_value_at_index(table, index);
}

void __impl_slot_delete(slot_t *table, const u64 index)
{
    if (table == NULL) eprint("table argument is null");

    ASSERT(index >= 0 && index < table->internal.capacity);

    table->internal.index_table[index] = false;

    table->len--;
}


void __impl_slot_destroy(slot_t *table)
{
    if (table == NULL) eprint("table arguemnt is null");

    if (table->internal.arena) {
        arena_giveback(table->internal.arena, table->data, table->internal.elem_size * table->internal.capacity);
        arena_giveback(table->internal.arena, table->internal.index_table, table->internal.capacity);
    } else {
        free(table->data);
        free(table->internal.index_table);
    }
    table->data = NULL;
    table->internal.index_table = NULL;
}

void slot_print(const slot_t *table, void (*print)(void*))
{
    ASSERT(table);
    ASSERT(print);

    printf("{");

    for (u64 i = 0; i < table->internal.capacity; i++)
    {
        if (!table->internal.index_table[i]) continue;

        print(slot_get_value(table, i));

        printf("\n");
    }
    printf("}\n");

}


void * slot_get_value(const slot_t *table, const u64 index)
{
    ASSERT(table);
    if(index <= 0 && index >= table->internal.capacity) eprint("invalid index (%li) value", index);;
    ASSERT(table->len > 0);

    return __slot_get_reference_to_only_value_at_index(table, index);
}

void slot_dump(const slot_t *table)
{
    ASSERT(table);

    printf("len = %li\ncapacity = %li\nelem_size = %li\nare_elem_are_pointers = %s\n",
         table->len,
         table->internal.capacity,
         table->internal.elem_size,
         table->internal.stored_as_pointers ? "TRUE" : "FALSE");

    const char *output = 
        "=====================\n"
        "    INDEX | OCCUPIED \n"
        "=====================\n";

    printf("%s", output);
    for (u64 i = 0; i < table->internal.capacity; i++)
    {
        printf("%02li | %s \n", i, table->internal.index_table[i] ? "TRUE" : "FALSE");
    }
    printf("=====================\n");
}

void slot_clear(slot_t *slot)
{
    ASSERT(slot);
    slot->len = 0;
    memset(slot->internal.index_table, 0, sizeof(bool) * slot->internal.capacity);
}

slot_t slot_clone(const slot_t *slot)
{
    slot_t output = slot_init(
            slot->internal.capacity, 
            slot->internal.elem_size,
            slot->internal.stored_as_pointers,
            slot->internal.arena);

    slot_iterator(slot, iter) {
        slot_insert(&output,
            output.len, 
            iter, 
            slot->internal.elem_size);
    }

    return output;
}

void slot_insert_multiple(
        slot_t *slot, 
        const u8 *arr, 
        const u32 arrlen, 
        const u32 elem_size)
{
    ASSERT(slot);
    ASSERT(arr);
    ASSERT(arrlen > 0);
    ASSERT(elem_size > 0);

    for (u32 i = 0; i < arrlen; i++)
    {
        slot_insert(
                slot, 
                slot->len, 
                arr + (elem_size * i), 
                elem_size);
    }
}

#endif

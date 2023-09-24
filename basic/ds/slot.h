#pragma once
#include "../common.h"
#include "./list.h"

/*============================================================================
                - STATIC ARRAY (SLOT ARRAY ) DATA STRUCTURE -
============================================================================*/

typedef struct slot_t {

    // public 
    u64                 len;

    //private
    u8                  *__data;
    char                __elem_type[MAX_TYPE_CHARACTER_LENGTH];
    u64                 __elem_size;
    u64                 __capacity;
    bool                *__index_table;
    bool                __are_elem_pointers;

} slot_t ;


#define             slot_init(CAPACITY, TYPE)                              __impl_slot_init((CAPACITY), (#TYPE), sizeof(TYPE))
#define             slot_insert(PSLOTARRAY, INDEX, VALUE)                  __impl_slot_insert((PSLOTARRAY), (INDEX), &(VALUE), sizeof(VALUE))
#define             slot_append(PSLOTARRAY, VALUE)                         slot_insert((PSLOTARRAY), (PSLOTARRAY)->len, (VALUE))
#define             slot_delete(PSLOTARRAY, INDEX)                         __impl_slot_delete((PSLOTARRAY), (INDEX))
slot_t              slot_clone(const slot_t *slot);

void *              slot_get_value(const slot_t *table, const u64 index);
#define             slot_iterator(PSLOTARRAY, ITER)                        __impl_slot_for_loop_iterator((PSLOTARRAY), (ITER))
#define             slot_get_capacity(PSLOT) (PSLOT)->__capacity
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
    while(*index < slot->__capacity && *hits < slot->len) {

        if (slot->__index_table[*index]) {
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


slot_t __impl_slot_init(const u64 array_capacity, const char *elem_type, const u64 elem_size)
{
    assert(elem_type);
    assert(elem_size > 0);

    bool flag = false;
    u32 len = strlen(elem_type);
    if (elem_type[len] > 15) eprint("variable name is too big, exceeded the 16 limit threshold\n");
    if (elem_type[len - 1] == '*') flag = true;

    slot_t o = {
        .len = 0,
        .__data               = (u8 *)calloc(array_capacity, elem_size),
        .__elem_type           = {0},
        .__elem_size           = elem_size,
        .__capacity             = array_capacity,
        .__index_table          = (bool *)calloc(array_capacity, sizeof(bool)),
        .__are_elem_pointers  = flag,
    };

    if (!flag)  memcpy(o.__elem_type, elem_type, MAX_TYPE_CHARACTER_LENGTH);
    else        memcpy(o.__elem_type, elem_type, len - 1);

    return o;
}

bool __check_if_empty(const slot_t *table, const u64 index)
{
    return table->__index_table[index];
}

void * __slot_get_reference_to_only_value_at_index(const slot_t *table, const u64 index)
{
    if (!table->__are_elem_pointers)
        return (void *)(table->__data + index * table->__elem_size);
    else 
        return *(void **)(table->__data + index * table->__elem_size);
}


void * __impl_slot_insert(
        slot_t *table,
        const u64   index, 
        const void  *value_addr, 
        const u64   value_size)
{ 
    if (table == NULL) eprint("table argument is null");
    if (value_size != table->__elem_size) eprint("expected value size (%li) but got (%li)", table->__elem_size, value_size);
    assert(index >= 0 && index < table->__capacity);

    if (!__check_if_empty(table, index)) {

        memcpy(
            table->__data + (index * table->__elem_size), 
            value_addr, 
            table->__elem_size);

        table->__index_table[index] = true;

    } else {

        eprint("slot at [%li] index is not empty", index);

    } 
    table->len++;
    
    return __slot_get_reference_to_only_value_at_index(table, index);
}

void __impl_slot_delete(slot_t *table, const u64 index)
{
    if (table == NULL) eprint("table argument is null");

    assert(index >= 0 && index < table->__capacity);

    table->__index_table[index] = false;

    table->len--;
}


void __impl_slot_destroy(slot_t *table)
{
    if (table == NULL) eprint("table arguemnt is null");

    free(table->__data);
    table->__data = NULL;
    free(table->__index_table);
    table->__index_table = NULL;
}

void slot_print(const slot_t *table, void (*print)(void*))
{
    assert(table);
    assert(print);

    printf("{");

    for (u64 i = 0; i < table->__capacity; i++)
    {
        if (!table->__index_table[i]) continue;

        print(slot_get_value(table, i));

        printf("\n");
    }
    printf("}\n");

}


void * slot_get_value(const slot_t *table, const u64 index)
{
    assert(table);
    if(index <= 0 && index >= table->__capacity) eprint("invalid index (%li) value", index);;

    return __slot_get_reference_to_only_value_at_index(table, index);
}

void slot_dump(const slot_t *table)
{
    assert(table);

    printf("len = %li\ncapacity = %li\nelem_size = %li\nelem_type = %s\nare_elem_are_pointers = %s\n",
         table->len,
         table->__capacity,
         table->__elem_size,
         table->__elem_type,
         table->__are_elem_pointers ? "TRUE" : "FALSE");

    const char *output = 
        "=====================\n"
        "    INDEX | OCCUPIED \n"
        "=====================\n";

    printf("%s", output);
    for (u64 i = 0; i < table->__capacity; i++)
    {
        printf("%02li | %s \n", i, table->__index_table[i] ? "TRUE" : "FALSE");
    }
    printf("=====================\n");
}

void slot_clear(slot_t *slot)
{
    assert(slot);
    slot->len = 0;
    memset(slot->__index_table, 0, sizeof(bool) * slot->__capacity);
}

slot_t slot_clone(const slot_t *slot)
{
    slot_t output = __impl_slot_init(
            slot->__capacity, 
            slot->__elem_type, 
            slot->__elem_size);

    slot_iterator(slot, iter) {
        __impl_slot_insert(&output,
            output.len, 
            iter, 
            slot->__elem_size);
    }

    return output;
}
#endif

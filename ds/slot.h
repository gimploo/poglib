#pragma once
#include "../basic.h"
#include "../str.h"
#include "./list.h"

/*============================================================================
                - STATIC ARRAY (SLOT ARRAY ) DATA STRUCTURE -
============================================================================*/

typedef struct slot_t {

    // public 
    u64                 len;

    //private
    u8                  *__values;
    char                *__value_type;
    u64                 __value_size;
    u64                 __capacity;
    bool                *__index_table;
    bool                __are_values_pointers;

} slot_t ;


#define             slot_init(CAPACITY, TYPE)                              __impl_slot_init((CAPACITY), (#TYPE), sizeof(TYPE))
#define             slot_insert(PSLOTARRAY, INDEX, VALUE)                  __impl_slot_insert((PSLOTARRAY), (INDEX), &(VALUE), sizeof(VALUE))
#define             slot_append(PSLOTARRAY, VALUE)                         slot_insert((PSLOTARRAY), (PSLOTARRAY)->len, (VALUE))
#define             slot_delete(PSLOTARRAY, INDEX)                         __impl_slot_delete((PSLOTARRAY), (INDEX))

void *              slot_get_value(const slot_t *table, const u64 index);
#define             slot_iterator(PSLOTARRAY, ITER)                        __impl_slot_for_loop_iterator((PSLOTARRAY), (ITER))

void                slot_print(const slot_t *table, void (*print)(void*));
void                slot_dump(const slot_t *table);

#define             slot_destroy(PSLOTARRAY)                               __impl_slot_destroy(PSLOTARRAY)



#ifndef IGNORE_SLOTARRAY_IMPLEMENTATION

/*------------------------------------------------------------------------------
                             IMPLEMENTATION 
------------------------------------------------------------------------------*/

#define __impl_slot_for_loop_iterator(PSLOTARRAY, TMP)\
                if ((PSLOTARRAY)->len != 0)\
                    for (void *index = 0, *(TMP) = (void *)slot_get_value((PSLOTARRAY), 0);\
                             (u64)(index) < (PSLOTARRAY)->len;\
                             index = (void *)((u64)index + 1),\
                             (TMP) = (void *)slot_get_value((PSLOTARRAY), ((u64)index < (PSLOTARRAY)->len) ? (u64)index : (u64)((u64)index - 1)))


slot_t __impl_slot_init(const u64 array_capacity, const char *elem_type, const u64 elem_size)
{
    bool flag = false;
    if (elem_type[strlen(elem_type) - 1] == '*') flag = true;

    slot_t o = {
        .len = 0,
        .__values               = (u8 *)calloc(array_capacity, elem_size),
        .__value_type           = (char *)elem_type,
        .__value_size           = elem_size,
        .__capacity             = array_capacity,
        .__index_table          = (bool *)calloc(array_capacity, sizeof(bool)),
        .__are_values_pointers  = flag,
    };

    return o;
}

bool __check_if_empty(const slot_t *table, const u64 index)
{
    return table->__index_table[index];
}

void * __slot_get_reference_to_only_value_at_index(const slot_t *table, const u64 index)
{
    if (!table->__are_values_pointers)
        return (void *)(table->__values + index * table->__value_size);
    else 
        return *(void **)(table->__values + index * table->__value_size);
}


void * __impl_slot_insert(
        slot_t *table,
        const u64   index, 
        const void  *value_addr, 
        const u64   value_size)
{ 
    if (table == NULL) eprint("table argument is null");
    if (value_size != table->__value_size) eprint("expected value size (%li) but got (%li)", table->__value_size, value_size);
    assert(index >= 0 && index < table->__capacity);

    if (!__check_if_empty(table, index)) {

        memcpy(
            table->__values + (index * table->__value_size), 
            value_addr, 
            table->__value_size);

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

    free(table->__values);
    free(table->__index_table);
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

    printf("len = %li\ncapacity = %li\nelem_size = %li\nelem_type = %s\nare_values_are_pointers = %s\n",
         table->len,
         table->__capacity,
         table->__value_size,
         table->__value_type,
         table->__are_values_pointers ? "TRUE" : "FALSE");

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
#endif

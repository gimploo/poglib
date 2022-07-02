#pragma once
#include "basic.h"
#include "./ds/stack.h"
#include "./ds/queue.h"
#include "./ds/list.h"
#include "./ds/hashtable.h"
#include "./ds/map.h"
#include "./ds/linkedlist.h"
#include "./ds/slot.h"
#include "file.h"

//TODO: load ds part is left

typedef enum {
    DS_stack_t,
    DS_queue_t,
    DS_list_t,
    DS_hashtable_t,
    DS_map_t,
    DS_linkedlist_t,
    DS_slot_t,
    DS_COUNT
} ds_type;

#define DS_Type(TYPE) DS_##TYPE

void __impl_file_save_ds(file_t *file, void *ds, ds_type type)
{
    assert(!file->is_closed);

    switch(type)
    {
        case DS_stack_t: 
            file_writebytes(file, (stack_t *)ds, sizeof(stack_t ));
            file_writebytes(
                    file, 
                    ((stack_t *)ds)->__array, 
                    ((stack_t *)ds)->__capacity * ((stack_t *)ds)->__elem_size);
        break;

        case DS_queue_t: 
            file_writebytes(file, (queue_t *)ds, sizeof(queue_t ));
            file_writebytes(
                    file, 
                    ((queue_t *)ds)->__array, 
                    ((queue_t *)ds)->__capacity * ((queue_t *)ds)->__elem_size);
        break;

        case DS_list_t: 
            file_writebytes(file, (list_t *)ds, sizeof(list_t ));
            file_writebytes(
                    file, 
                    ((list_t *)ds)->__array, 
                    ((list_t *)ds)->__capacity * ((list_t *)ds)->__elem_size);
        break;

        case DS_hashtable_t: 
            file_writebytes(file, (hashtable_t *)ds, sizeof(hashtable_t ));
            file_writebytes(
                    file, 
                    ((hashtable_t *)ds)->__values,
                    ((hashtable_t *)ds)->__capacity * ((hashtable_t *)ds)->__value_size);
        break;

        case DS_map_t: 
            file_writebytes(file, (map_t *)ds, sizeof(map_t ));
            file_writebytes(
                    file, 
                    ((map_t *)ds)->__keys.__array, 
                    ((map_t *)ds)->__keys.__capacity * ((map_t *)ds)->__keys.__elem_size);
            file_writebytes(
                    file, 
                    ((map_t *)ds)->__values.__values,
                    ((map_t *)ds)->__values.__capacity * ((map_t *)ds)->__values.__value_size);
        break;

        case DS_slot_t: 
            file_writebytes(file, (slot_t *)ds, sizeof(slot_t ));
            file_writebytes(
                    file,
                    ((slot_t *)ds)->__values, 
                    ((slot_t *)ds)->__capacity * ((slot_t *)ds)->__value_size);
        break;

        case DS_linkedlist_t: 
        default: eprint("type not accoutned for");
    }
}

void __impl_file_load_ds(file_t *file, void *ds, ds_type type)
{
    assert(ds);
    assert(!file->is_closed);

    switch(type)
    {
        case DS_stack_t: 
            file_readbytes(file, (stack_t *)ds, sizeof(stack_t ));
            file_readbytes(
                    file, 
                    ((stack_t *)ds)->__array, 
                    ((stack_t *)ds)->__capacity * ((stack_t *)ds)->__elem_size);
        break;

        case DS_queue_t: 
            file_readbytes(file, (queue_t *)ds, sizeof(queue_t ));
            file_readbytes(
                    file, 
                    ((queue_t *)ds)->__array, 
                    ((queue_t *)ds)->__capacity * ((queue_t *)ds)->__elem_size);
        break;

        case DS_list_t: 
            file_readbytes(file, (list_t *)ds, sizeof(list_t ));
            file_readbytes(
                    file, 
                    ((list_t *)ds)->__array, 
                    ((list_t *)ds)->__capacity * ((list_t *)ds)->__elem_size);
        break;

        case DS_hashtable_t: 
            file_readbytes(file, (hashtable_t *)ds, sizeof(hashtable_t ));
            file_readbytes(
                    file, 
                    ((hashtable_t *)ds)->__values,
                    ((hashtable_t *)ds)->__capacity * ((hashtable_t *)ds)->__value_size);
        break;

        case DS_map_t: 
            file_readbytes(file, (map_t *)ds, sizeof(map_t ));
            file_readbytes(
                    file, 
                    ((map_t *)ds)->__keys.__array, 
                    ((map_t *)ds)->__keys.__capacity * ((map_t *)ds)->__keys.__elem_size);
            file_readbytes(
                    file, 
                    ((map_t *)ds)->__values.__values,
                    ((map_t *)ds)->__values.__capacity * ((map_t *)ds)->__values.__value_size);
        break;

        case DS_slot_t: 
            file_readbytes(file, (slot_t *)ds, sizeof(slot_t ));
            file_readbytes(
                    file,
                    ((slot_t *)ds)->__values, 
                    ((slot_t *)ds)->__capacity * ((slot_t *)ds)->__value_size);
        break;

        case DS_linkedlist_t: 
        default: eprint("type not accoutned for");
    }

}


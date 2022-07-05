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
#include <string.h>

#define file_save_stack(PFILE, INPUT)\
    __impl_file_save_ds((PFILE), &(INPUT), sizeof(INPUT), DS_Type(stack_t ))

#define file_save_queue(PFILE, INPUT)\
    __impl_file_save_ds((PFILE), &(INPUT), sizeof(INPUT), DS_Type(queue_t ))

#define file_save_list(PFILE, INPUT)\
    __impl_file_save_ds((PFILE), &(INPUT), sizeof(INPUT), DS_Type(list_t ))

#define file_save_hashtable(PFILE, INPUT)\
    __impl_file_save_ds((PFILE), &(INPUT), sizeof(INPUT), DS_Type(hashtable_t ))

#define file_save_map(PFILE, INPUT)\
    __impl_file_save_ds((PFILE), &(INPUT), sizeof(INPUT), DS_Type(map_t ))

#define file_save_slot(PFILE, INPUT)\
    __impl_file_save_ds((PFILE), &(INPUT), sizeof(INPUT), DS_Type(slot_t ))

#define file_load_stack(PFILE) \
    __impl_file_load_ds((PFILE), DS_Type(stack_t))s

#define file_load_queue(PFILE) \
    __impl_file_load_ds((PFILE),DS_Type(queue_t)).q

#define file_load_list(PFILE) \
    __impl_file_load_ds((PFILE),DS_Type(list_t)).l

#define file_load_hashtable(PFILE) \
    __impl_file_load_ds((PFILE),DS_Type(hashtable_t)).h

#define file_load_map(PFILE) \
    __impl_file_load_ds((PFILE),DS_Type(map_t)).m

#define file_load_slot(PFILE) \
    __impl_file_load_ds((PFILE),DS_Type(slot_t)).sl


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

void __impl_file_save_ds(file_t *file, void *ds, u64 ds_size, ds_type type)
{
    assert(!file->is_closed);
    if (ds_size == 8) eprint("expected a non pointer");

    switch(type)
    {
        case DS_stack_t: 
            assert(ds_size == sizeof(stack_t ));
            file_writebytes(file, ds, sizeof(stack_t ));
            file_writebytes(
                    file, 
                    ((stack_t *)ds)->__array, 
                    ((stack_t *)ds)->__capacity * ((stack_t *)ds)->__elem_size);
        break;

        case DS_queue_t: 
            assert(ds_size == sizeof(queue_t ));
            file_writebytes(file, ds, sizeof(queue_t ));
            file_writebytes(
                    file, 
                    ((queue_t *)ds)->__array, 
                    ((queue_t *)ds)->__capacity * ((queue_t *)ds)->__elem_size);
        break;

        case DS_list_t: 
            assert(ds_size == sizeof(list_t ));
            file_writebytes(file, ds, sizeof(list_t ));
            file_writebytes(
                    file, 
                    ((list_t *)ds)->__array, 
                    ((list_t *)ds)->__capacity * ((list_t *)ds)->__elem_size);
        break;

        case DS_hashtable_t: 
            assert(ds_size == sizeof(hashtable_t ));
            file_writebytes(file, ds, sizeof(hashtable_t ));
            file_writebytes(
                    file, 
                    ((hashtable_t *)ds)->__values,
                    ((hashtable_t *)ds)->__capacity * ((hashtable_t *)ds)->__value_size);
            file_writebytes(
                    file, 
                    ((hashtable_t *)ds)->__index_table,
                    ((hashtable_t *)ds)->__capacity * sizeof(bool));
        break;

        case DS_map_t: 
            assert(ds_size == sizeof(map_t ));
            file_writebytes(file, ds, sizeof(map_t ));
            file_writebytes(
                    file, 
                    ((map_t *)ds)->__keys.__array, 
                    ((map_t *)ds)->__keys.__capacity * ((map_t *)ds)->__keys.__elem_size);
            file_writebytes(
                    file, 
                    ((map_t *)ds)->__values.__values,
                    ((map_t *)ds)->__values.__capacity * ((map_t *)ds)->__values.__value_size);
            file_writebytes(
                    file, 
                    ((map_t *)ds)->__values.__index_table,
                    ((map_t *)ds)->__values.__capacity * sizeof(bool));
        break;

        case DS_slot_t: 
            assert(ds_size == sizeof(slot_t ));
            file_writebytes(file, ds, sizeof(slot_t ));
            file_writebytes(
                    file,
                    ((slot_t *)ds)->__values, 
                    ((slot_t *)ds)->__capacity * ((slot_t *)ds)->__value_size);
            file_writebytes(
                    file,
                    ((slot_t *)ds)->__index_table, 
                    ((slot_t *)ds)->__capacity * sizeof(bool));
        break;

        case DS_linkedlist_t: 
        default: eprint("type not accoutned for");
    }
}

typedef union {
    stack_t     s;
    queue_t     q;
    llist_t     ll;
    list_t      l;
    hashtable_t h;
    map_t       m;
    slot_t      sl;     
} __ds_t;

__ds_t __impl_file_load_ds(file_t *file, ds_type type)
{
    assert(!file->is_closed);

    u8 ds[sizeof(__ds_t)] = {0};
    __ds_t output = {0};
    void *tmps[3] = {0};

    switch(type) 
    {
        case DS_stack_t: 
            file_readbytes(file, ds, sizeof(stack_t ));
            output.s = __impl_stack_init(
                    ((stack_t *)ds)->__capacity, 
                    ((stack_t *)ds)->__elem_type, 
                    ((stack_t *)ds)->__elem_size);
            file_readbytes(
                    file, 
                    output.s.__array, 
                    output.s.__capacity * output.s.__elem_size);
        break;

        case DS_queue_t: 
            file_readbytes(file, ds, sizeof(queue_t ));
            output.q = __impl_queue_init(
                    ((queue_t *)ds)->__capacity, 
                    ((queue_t *)ds)->__elem_size,
                    ((queue_t *)ds)->__elem_type);
            file_readbytes(
                    file, 
                    output.q.__array, 
                    output.q.__capacity * output.q.__elem_size);
        break;

        case DS_list_t: 
            file_readbytes(file, ds, sizeof(list_t ));
            output.l = __impl_list_init(
                    ((list_t *)ds)->__capacity, 
                    ((list_t *)ds)->__elem_type,
                    ((list_t *)ds)->__elem_size);
            {
                tmps[0] = output.l.__array;
                memcpy(&output.l, ds, sizeof(list_t ));
                output.l.__array = (u8 *)tmps[0];
            }
            file_readbytes(
                    file, 
                    output.l.__array, 
                    output.l.__capacity * output.l.__elem_size);
        break;

        case DS_hashtable_t: 
            file_readbytes(file, ds, sizeof(hashtable_t ));
            output.h = __impl_hashtable_init(
                    ((hashtable_t *)ds)->__capacity, 
                    ((hashtable_t *)ds)->__value_type,
                    ((hashtable_t *)ds)->__value_size);
            {
                tmps[0] = output.h.__values;
                tmps[1] = output.h.__index_table;
                memcpy(&output.h, ds, sizeof(hashtable_t ));
                output.h.__values = (u8 *)tmps[0];
                output.h.__index_table = (bool *)tmps[1];
            }
            file_readbytes(
                    file, 
                    output.h.__values,
                    output.h.__capacity * output.h.__value_size);
            file_readbytes(
                    file, 
                    output.h.__index_table,
                    output.h.__capacity * sizeof(bool));
        break;

        case DS_map_t: 
            file_readbytes(file, ds, sizeof(map_t ));
            output.m = __impl_map_init(
                    ((map_t *)ds)->__values.__capacity, 
                    ((map_t *)ds)->__values.__value_type,
                    ((map_t *)ds)->__values.__value_size);

            {   
                tmps[0] = output.m.__keys.__array;
                tmps[1] = output.m.__values.__values;
                tmps[2] = output.m.__values.__index_table;
                memcpy(&output.m, ds, sizeof(map_t ));
                output.m.__keys.__array         = (u8 *)tmps[0];
                output.m.__values.__values      = (u8 *)tmps[1];
                output.m.__values.__index_table = (bool *)tmps[2];
            }

            file_readbytes(
                    file, 
                    output.m.__keys.__array, 
                    output.m.__keys.__capacity * output.m.__keys.__elem_size);
            file_readbytes(
                    file, 
                    output.m.__values.__values,
                    output.m.__values.__capacity * output.m.__values.__value_size);
            file_readbytes(
                    file, 
                    output.m.__values.__index_table,
                    output.m.__values.__capacity * sizeof(bool));
        break;

        case DS_slot_t: 
            file_readbytes(file, ds, sizeof(slot_t ));
            output.sl = __impl_slot_init(
                    ((slot_t *)ds)->__capacity, 
                    ((slot_t *)ds)->__value_type,
                    ((slot_t *)ds)->__value_size);
            {
                tmps[0] = output.sl.__values;
                tmps[1] = output.sl.__index_table;
                memcpy(&output.sl, ds, sizeof(slot_t ));
                output.sl.__values = (u8 *)tmps[0];
                output.sl.__index_table = (bool *)tmps[1];
            }
            file_readbytes(
                    file,
                    output.sl.__values, 
                    output.sl.__capacity * output.sl.__value_size);
            file_readbytes(
                    file,
                    output.sl.__index_table, 
                    output.sl.__capacity * sizeof(bool));
        break;

        case DS_linkedlist_t: 
        default: eprint("type not accoutned for");
    }

    return output;
}


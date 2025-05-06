#pragma once
#include "dbg.h"
#include "common.h"
#include "file.h"
#include "ds/stack.h"
#include "ds/queue.h"
#include "ds/list.h"
#include "ds/hashtable.h"
#include "ds/linkedlist.h"
#include "ds/slot.h"

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

#define file_load_slot(PFILE) \
    __impl_file_load_ds((PFILE),DS_Type(slot_t)).sl


typedef enum {
    DS_stack_t,
    DS_queue_t,
    DS_list_t,
    DS_hashtable_t,
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
                    ((stack_t *)ds)->__data, 
                    ((stack_t *)ds)->__capacity * ((stack_t *)ds)->__elem_size);
        break;

        case DS_queue_t: 
            assert(ds_size == sizeof(queue_t ));
            file_writebytes(file, ds, sizeof(queue_t ));
            file_writebytes(
                    file, 
                    ((queue_t *)ds)->__data, 
                    ((queue_t *)ds)->__capacity * ((queue_t *)ds)->__elem_size);
        break;

        case DS_list_t: 
            assert(ds_size == sizeof(list_t ));
            file_writebytes(file, ds, sizeof(list_t ));
            file_writebytes(
                    file, 
                    ((list_t *)ds)->__data, 
                    ((list_t *)ds)->__capacity * ((list_t *)ds)->__elem_size);
        break;

        case DS_hashtable_t: 
            eprint("Not implemented");
        break;

        case DS_slot_t: 
            assert(ds_size == sizeof(slot_t ));
            file_writebytes(file, ds, sizeof(slot_t ));
            file_writebytes(
                    file,
                    ((slot_t *)ds)->__data, 
                    ((slot_t *)ds)->__capacity * ((slot_t *)ds)->__elem_size);
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
                    output.s.__data, 
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
                    output.q.__data, 
                    output.q.__capacity * output.q.__elem_size);
        break;

        case DS_list_t: 
            file_readbytes(file, ds, sizeof(list_t ));
            output.l = __impl_list_init(
                    ((list_t *)ds)->__capacity, 
                    ((list_t *)ds)->__elem_type,
                    ((list_t *)ds)->__elem_size);
            {
                tmps[0] = output.l.__data;
                memcpy(&output.l, ds, sizeof(list_t ));
                output.l.__data = (u8 *)tmps[0];
            }
            file_readbytes(
                    file, 
                    output.l.__data, 
                    output.l.__capacity * output.l.__elem_size);
        break;

        case DS_hashtable_t: 
            eprint("Not implemented");
        break;

        case DS_slot_t: 
            file_readbytes(file, ds, sizeof(slot_t ));
            output.sl = __impl_slot_init(
                    ((slot_t *)ds)->__capacity, 
                    ((slot_t *)ds)->__elem_type,
                    ((slot_t *)ds)->__elem_size);
            {
                tmps[0] = output.sl.__data;
                tmps[1] = output.sl.__index_table;
                memcpy(&output.sl, ds, sizeof(slot_t ));
                output.sl.__data = (u8 *)tmps[0];
                output.sl.__index_table = (bool *)tmps[1];
            }
            file_readbytes(
                    file,
                    output.sl.__data, 
                    output.sl.__capacity * output.sl.__elem_size);
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


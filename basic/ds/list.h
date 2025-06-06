#pragma once
#include "../dbg.h"
#include "../common.h"

/*==============================================================================
                        - DYNAMIC ARRAY DATA STRUCTURE -
==============================================================================*/

typedef struct list_t {

    u64     len;                        // length of the list
    u8      *data;
    u64     __capacity;
    i64     __top;
    u64     __elem_size;
    char    __elem_type[MAX_TYPE_CHARACTER_LENGTH];
    u64     __original_capacity;
    bool    __are_values_pointers;     // This variable checks if the list is a list of pointers 
                                       
} list_t ;

#define DEFAULT_LIST_STARTING_CAPACITY 4

#define         list_init(TYPE)                                 __impl_list_init(DEFAULT_LIST_STARTING_CAPACITY, #TYPE, sizeof(TYPE))

#define         list_append(PLIST, VALUE)                       __impl_list_append((PLIST), &(VALUE), sizeof(VALUE)) 
void            list_delete(list_t *list, const u64 index);
#define         list_clear(PLIST)                               __impl_list_clear(PLIST)
void            list_combine(list_t *dest, const list_t *src);
#define         list_get_size(PLIST)                             ((PLIST)->__elem_size * (PLIST)->len)
#define         list_append_multiple(PLIST, ARRAY)              __impl_list_append_multiple((PLIST), (u8*)(ARRAY), sizeof((ARRAY)), sizeof((*ARRAY)))

void            list_dump(const list_t *list);
void            list_print(const list_t *list, void (*print)(void*));

#define         list_get_buffer(PLIST) (PLIST)->data
void *          list_get_value(const list_t *list, const u64 index);
#define         list_iterator(PLIST, ITER)                          __impl_list_iterator((PLIST), (ITER), list_index)

void            list_destroy(list_t *list);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION 
-----------------------------------------------------------------------------*/

#ifndef IGNORE_LIST_IMPLEMENTATION

#define __impl_list_iterator(PLIST, ITER, IDX)\
    if ((PLIST)->len != 0)\
        for (void **(IDX) = 0, *(ITER) = (void *)list_get_value((PLIST), (u64)(IDX));\
            (u64)(IDX) < (PLIST)->len;\
            (IDX) = (void **)((u64)(IDX) + 1),\
                (ITER) = (void *)list_get_value(PLIST, ((u64)(IDX) < (PLIST)->len ? (u64)(IDX) : (u64)(IDX)-1)))

void * list_get_value(const list_t *list, const u64 index)
{
    assert(index < list->len);

    if (list->__are_values_pointers)
        return *(void **)(list->data + index * list->__elem_size);
    else 
        return (list->data + index * list->__elem_size);
}

void __impl_list_clear(list_t *list)
{
    assert(list);
    list->__top = -1;
    list->len = 0;
    memset(list->data, 0, list->__elem_size * list->__capacity);

}

list_t __impl_list_init(const u64 capacity, const char *elem_type, const u64 elem_size) 
{
    assert(elem_type);
    assert(elem_size > 0);

    bool flag = false;
    u32 len = strlen(elem_type);
    if (elem_type[len] > 15) eprint("variable name is too big, exceeded the 16 limit threshold\n");
    if (elem_type[len - 1] == '*') flag = true;

    list_t o = {

        .len = 0,
        .__capacity = capacity,
        .data = (u8 *)calloc(capacity, elem_size),
        .__top = -1,
        .__elem_size = elem_size,
        .__elem_type = {0},
        .__original_capacity = capacity,
        .__are_values_pointers = flag,
    };

    if (!flag)  memcpy(o.__elem_type, elem_type, MAX_TYPE_CHARACTER_LENGTH);
    else        memcpy(o.__elem_type, elem_type, len - 1);

    return o;
}

void __impl_list_append(list_t *list, const void *value_addr, u64 value_size)
{
    assert(list);
    assert(value_addr);
    if (value_size != list->__elem_size) eprint("trying to push a value of size %lu to slot of size %lu", value_size, list->__elem_size);

    if (list->__top == (i64)(list->__capacity - 1)) {

        list->__capacity = list->__capacity * 2;
        list->data = (u8 *)realloc(list->data, list->__capacity * list->__elem_size);
        //printf("Grew to %ld\n", list->__capacity);
    }

    list->len = ++list->__top + 1;

    memcpy(list->data + list->__top * list->__elem_size, value_addr, list->__elem_size);
}


//TODO: check this for edge conditions, it looks like its could possibly fail
//in some common conditions
void list_delete(list_t *list, const u64 index)
{
    assert(list);
    if(list->__top == -1)           eprint("Trying to delete an element from an empty list\n");
    if((i64)index > list->__top)    eprint("index (`%lu`) exceeds list length (`%lu`) ", index, list->__top);

    if ((i64)index != list->__top) {

        memcpy(list->data + index * list->__elem_size, 
                list->data + (index + 1) * list->__elem_size, 
                list->__elem_size * (list->__top - index)); 
    } 

    list->len = --list->__top + 1;

    if ((list->__capacity != list->__original_capacity) 
            && (list->__top == (i64)((list->__capacity / 2) - 1))) { 

        list->__capacity = list->__capacity / 2;
        list->data = (u8 *)realloc(list->data, list->__capacity * list->__elem_size);
        printf("Shrunk from %ld to %ld\n", list->__capacity * 2, list->__capacity);
    }
} 

void list_print(const list_t *list, void (*print)(void*))
{
    assert(list);
    if (list->__top == -1) {
        printf("[]\n");
        return ;
    }
    printf("[ ");
    for (u64 i = 0; i < list->len; i++)
    {
        if (list->__are_values_pointers)
            print(*(void **)(list->data + i * list->__elem_size));
        else 
            print(list->data + i * list->__elem_size);
    }
    printf("]\n");
}

void list_dump(const list_t *list)
{
    assert(list);

    printf("\n len = %ld\n arr = %p\n top = %ld\n capacity = %ld\n elem_size = %ld\n", 
            list->len, 
            list->data, 
            list->__top, 
            list->__capacity, 
            list->__elem_size);

    printf(" contents = [ ");
    for (u64 i = 0; i < list->__capacity; i++)
    {
        printf("%p, ",list->data + i * list->__elem_size);
    }
    printf("]\n");

}

void list_destroy(list_t *list)
{
    free(list->data);
    list->data = NULL;
    list->__capacity = 0;
    list->__top = -1;
    list->len = 0;
    list->__elem_size = 0;
}

void list_combine(list_t *dest, const list_t *src)
{
    if (strcmp(dest->__elem_type, src->__elem_type) != 0)
        eprint("destination (%s) and src (%s) types are different", 
                dest->__elem_type, src->__elem_type);

    list_iterator(src, iter)
        __impl_list_append(dest, iter, dest->__elem_size);
}

void __impl_list_append_multiple(
        list_t *slot, 
        const u8 *arr, 
        const u64 arr_size, 
        const u32 elem_size)
{
    ASSERT(slot);
    ASSERT(arr);
    ASSERT(arr_size > 0);
    ASSERT(elem_size > 0);

    const u32 arr_len = (arr_size / elem_size);
    for (u32 i = 0; i < arr_len; i++)
    {
        __impl_list_append(
                slot, 
                arr + (elem_size * i), 
                elem_size);
    }
}
#endif

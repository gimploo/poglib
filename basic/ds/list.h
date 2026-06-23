#pragma once
#include "../dbg.h"
#include "../common.h"
#include "../arena.h"

//TODO: bring better cache locality

/*==============================================================================
                        - DYNAMIC ARRAY DATA STRUCTURE -
==============================================================================*/

typedef struct list_t {

    u64     len;                        // length of the list
    u8      *data;
    arena_t *arena;                     // NULL = malloc/free, non-NULL = arena allocator
    struct {
        u64     capacity;
        i64     top;
        u64     elem_size;
        u64     original_capacity;
    } internal;

} list_t ;

#define DEFAULT_LIST_STARTING_CAPACITY 4 

#define         list_init(TYPE, ARENA)                               list__internal__init(DEFAULT_LIST_STARTING_CAPACITY, #TYPE, sizeof(TYPE), ARENA)

#define         list_append(PLIST, VALUE)                       list__internal__append((PLIST), &(VALUE), sizeof(VALUE)) 
#define         list_append_ptr(PLIST, PVALUE)                   list__internal__append((PLIST), &(PVALUE), sizeof(void *)) 
void            list_delete(list_t *list, const u64 index);
void            list_clear(list_t *const list);
void            list_combine(list_t *dest, const list_t *src);
#define         list_get_size(PLIST)                             ((PLIST)->internal.elem_size * (PLIST)->len)
#define         list_append_multiple(PLIST, ARRAY)              list__internal__append_multiple((PLIST), (u8*)(ARRAY), sizeof((ARRAY)), sizeof((*ARRAY)))

void            list_dump(const list_t *list);
void            list_print(const list_t *list, void (*print)(void*));
bool            list_is_init(const list_t * const self);

#define         list_get_buffer(PLIST) (PLIST)->data
void *          list_get_value(const list_t *list, const u64 index);
#define         list_iterator(PLIST, ITER)                          list__internal__iterator((PLIST), (ITER), list_iterator_index)

void            list_destroy(list_t *list);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION 
-----------------------------------------------------------------------------*/

#ifndef IGNORE_LIST_IMPLEMENTATION

#define list__internal__iterator(PLIST, ITER, IDX)\
    if ((PLIST)->len != 0 && (PLIST)->internal.capacity != 0)\
        for (void **(IDX) = 0, *(ITER) = (void *)list_get_value((PLIST), (u64)(IDX));\
            (u64)(IDX) < (PLIST)->len;\
            (IDX) = (void **)((u64)(IDX) + 1),\
                (ITER) = (void *)list_get_value(PLIST, ((u64)(IDX) < (PLIST)->len ? (u64)(IDX) : (u64)(IDX)-1)))

void * list_get_value(const list_t *list, const u64 index)
{
    assert(index < list->len);

    return (list->data + index * list->internal.elem_size);
}

void list_clear(list_t *const list)
{
    assert(list);
    list->internal.top = -1;
    list->len = 0;
    memset(list->data, 0, list->internal.elem_size * list->internal.capacity);
}

list_t list__internal__init(const u64 capacity, const char *elem_type, const u64 elem_size, arena_t *const arena) 
{
    assert(elem_type);
    assert(elem_size > 0);

    u32 len = strlen(elem_type);
    if (len > MAX_TYPE_CHARACTER_LENGTH - 1) eprint("variable name is too big, exceeded the 16 limit threshold\n");

    list_t o = {
        .len = 0,
        .data = (u8 *)arena_reserve(arena, capacity * elem_size),
        .arena = arena,
        .internal = {
            .capacity = capacity,
            .top = -1,
            .elem_size = elem_size,
            .original_capacity = capacity,
        },
    };

    return o;
}

void list__internal__append(list_t *list, const void *value_addr, u64 value_size)
{
    assert(list);
    assert(value_addr);
    if (value_size != list->internal.elem_size) eprint("trying to push a value of size %lu to slot of size %lu", value_size, list->internal.elem_size);

    if (list->internal.top == (i64)(list->internal.capacity - 1)) {

        u64 new_capacity = list->internal.capacity * 2;
        u8 *new_data = arena_reserve(list->arena, new_capacity * list->internal.elem_size);
        memcpy(new_data, list->data, list->internal.capacity * list->internal.elem_size);
        list->data = new_data;
        list->internal.capacity = new_capacity;
    }

    list->len = ++list->internal.top + 1;

    memcpy(list->data + list->internal.top * list->internal.elem_size, value_addr, list->internal.elem_size);
}


//TODO: check this for edge conditions, it looks like its could possibly fail
//in some common conditions
void list_delete(list_t *list, const u64 index)
{
    assert(list);
    if(list->internal.top == -1)           eprint("Trying to delete an element from an empty list\n");
    if((i64)index > list->internal.top)    eprint("index (`%lu`) exceeds list length (`%lu`) ", index, list->internal.top);

    if ((i64)index != list->internal.top) {

        memcpy(list->data + index * list->internal.elem_size, 
                list->data + (index + 1) * list->internal.elem_size, 
                list->internal.elem_size * (list->internal.top - index)); 
    } 

    list->len = --list->internal.top + 1;

} 

void list_print(const list_t *list, void (*print)(void*))
{
    assert(list);
    if (list->internal.top == -1) {
        printf("[]\n");
        return ;
    }
    printf("[ ");
    for (u64 i = 0; i < list->len; i++)
    {
        print(list->data + i * list->internal.elem_size);
    }
    printf("]\n");
}

void list_dump(const list_t *list)
{
    assert(list);

    printf("\n len = %ld\n arr = %p\n top = %ld\n capacity = %ld\n elem_size = %ld\n", 
            list->len, 
            list->data, 
            list->internal.top, 
            list->internal.capacity, 
            list->internal.elem_size);

    printf(" contents = [ ");
    for (u64 i = 0; i < list->internal.capacity; i++)
    {
        printf("%p, ",list->data + i * list->internal.elem_size);
    }
    printf("]\n");

}

void list_destroy(list_t *list)
{
    list->data = NULL;
    list->internal.capacity = 0;
    list->internal.top = -1;
    list->len = 0;
    list->internal.elem_size = 0;

    arena_destroy(list->arena);
}

void list_combine(list_t *dest, const list_t *src)
{
    ASSERT(dest->internal.elem_size == src->internal.elem_size);

    list_iterator(src, iter)
        list__internal__append(dest, iter, dest->internal.elem_size);
}

void list__internal__append_multiple(
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
        list__internal__append(
                slot, 
                arr + (elem_size * i), 
                elem_size);
    }
}


bool list_is_init(const list_t *const self)
{
    return self->internal.original_capacity > 0;
}
#endif

#pragma once
#include "../basic.h"


typedef struct list_t list_t;


#define list_init(CAPACITY) __impl_list_init(CAPACITY)
#define list_append(PLIST, VALUE) __impl_list_append((PLIST), (VALUE)) 
void    list_delete(list_t *list, const u64 index);
void    list_free(list_t *list);

void list_dump(list_t *list);
void list_print(list_t *list, void (*print)(void*));

#define list_len(PLIST) (PLIST)->__top + 1



#ifndef IGNORE_LIST_IMPLEMENTATION

struct list_t {

    void **__array;
    i64 __top;
    u64 __capacity;
};


list_t __impl_list_init(const u64 capacity) 
{
    return (list_t)
    {
        .__array = (void **)calloc(capacity, sizeof(u64)),
        .__top = -1,
        .__capacity = capacity,
    };
}

void __impl_list_append(list_t *list, void *value)
{
    assert(list);
    assert(value);

    if (list->__top == (list->__capacity - 1)) {

        list->__capacity = list->__capacity * 2;
        void **tmp = (void **)realloc(list->__array, list->__capacity * sizeof(u64));
        assert(tmp);
        list->__array = tmp;
    }

    list->__array[++list->__top] = value;
}


void list_delete(list_t *list, const u64 index)
{
    assert(list);
    assert(list->__top != -1);
    assert(index >= 0);
    assert(index <= list->__top);


    if (index != list->__top) {
        memcpy(&list->__array[index], 
                &list->__array[index + 1], 
                sizeof(void *) * list->__top); 
    } 

    list->__array[list->__top] = NULL;

    --list->__top;

} 

void list_print(list_t *list, void (*print)(void*))
{
    assert(list);
    if (list->__top == -1) {
        printf("list is empty\n");
        return ;
    }
    for (u64 i = 0; i <= list->__top; i++)
    {
        print(list->__array[i]);
    }
    printf("\n");
}

void list_dump(list_t *list)
{
    assert(list);
    if (list->__top == -1) {
        printf("list is empty\n");
        return ;
    }
    for (u64 i = 0; i < list->__capacity; i++)
    {
        printf("%p ", list->__array[i]);
    }
    printf("\n");

}

void list_free(list_t *list)
{
    free(list->__array);
    list->__array = NULL;
    list->__capacity = 0;
    list->__top = -1;
}
#endif

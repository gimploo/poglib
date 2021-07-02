#ifndef _LIST_H_
#define _LIST_H_

#ifdef DEBUG
#include "dbg.h"
extern dbg_t debug;
#endif

#include "str.h"

typedef enum ValType {

    VT_PSTR,  
    VT_ADDR,
    VT_COUNT

} ValType;

typedef struct node {

    void * value;
    ValType type;
    struct node *next;
    struct node *prev;

} Node;

typedef struct {

    struct node *head;
    struct node *tail;
    size_t count;

} List;

static inline List list_init(void)
{
    return (List) {
        .head = NULL,
        .tail = NULL,
        .count = 0
    };
}



typedef void (*list_print_func)(void*);

Node *  node_init(void *value, ValType type);

bool    list_append_node(List *list, Node *node);
bool    list_delete_node(List *list, Node *node);
void    list_print(List *list);
void    list_destory(List *list);


/*
 *
 * FUNCTION DEFINITION -
 * ------------------- |
 *                     v
 */

Node * node_init(void *value, ValType type)
{

    Node *node = (Node *) malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        return NULL;
    }

    if (value == NULL) {
        fprintf(stderr, "%s: value argument is null\n", __func__);
        exit(1);
    }

    switch(type) {
        case VT_ADDR:
            node->value = value;
            node->type = VT_ADDR;
            break;
        case VT_PSTR:
            node->value = new_pstr((char *)value);
            node->type = VT_PSTR;
            break;
        default:
            fprintf(stderr, "%s: type not accounted for \n", __func__);
            exit(1);
    }

    return node;
}
bool list_append_node(List *list, Node *node)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        return false;
    }

    if (node == NULL) {
        fprintf(stderr, "%s: node argument is null\n", __func__);
        return false;
    }

    if (list->head == NULL) {

        list->head = node;
        list->tail = node;
        list->count++;
        return true;
    } 

    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
    list->count++;
    return true;
}

void node_destory(Node *del)
{
    switch(del->type) {
        case VT_PSTR: 
            str_free((str_t *)del->value);
            free(del);
            break;
        case VT_ADDR:
            free(del);
            break;
        default:
            fprintf(stderr, "%s: type not accounted for\n", __func__);
            exit(1);
    }
}



bool list_delete_node(List *list, Node *node)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        return false;
    }

    if (node == NULL) {
        fprintf(stderr, "%s: node argument is null\n", __func__);
        return false;
    }

    Node *del = NULL;
    if (list->head == node) {

        del = list->head; 
        list->head = list->head->next;
        list->head->prev = NULL;
        list->count--;
        node_destory(del);

    } else if (list->tail == node) {

        del = list->tail; 
        list->tail = list->tail->prev;
        list->head->next = NULL;
        list->count--;
        node_destory(del);
        return true;
    }

    Node *tmp = list->head->next;
    while (tmp != NULL) {
        if (tmp == node) {
            del = tmp;
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
            list->count--;
            node_destory(del);
            return true;
        }
        tmp = tmp->next;
    }

    return false;
}



void list_print(List *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        exit(1);
    } else if (list->head == NULL) {
        fprintf(stderr, "%s: list is empty\n", __func__);
        exit(1);
    }

    Node *track = list->head;
    while (track != NULL) {
        switch(track->type) {
            case VT_ADDR:
                printf("%p", track->value);
                break;
            case VT_PSTR:
                str_print((str_t *)track->value);
                break;
            default:
                fprintf(stderr, "%s: type not accounted for\n", __func__);
                exit(1);
        }
        track = track->next;
    }
}

void list_destory(List *list)
{
    if (list == NULL) {
        fprintf(stderr ,"%s: list argument is null\n", __func__);
        exit(1);
    }
    Node *tmp = list->head;
    Node *del = NULL;
    while (tmp != NULL) {
        del = tmp;
        tmp = tmp->next;
        node_destory(del);
    }
}


#endif //_LIST_H_

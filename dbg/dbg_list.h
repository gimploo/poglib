#ifndef _DBG_LIST_H_
#define _DBG_LIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 *
 * Same version of the other list.h but to have the other list use 
 * dbg.h, the best solution i know now is to create a separate list header
 * just for this dbg
 *
 */


typedef enum DValType {

    DVT_PSTR,  
    DVT_ADDR,
    DVT_COUNT

} DValType;

typedef struct DNode {

    void * value;
    DValType type;
    struct DNode *next;
    struct DNode *prev;

} DNode;

typedef struct {

    struct DNode *head;
    struct DNode *tail;
    size_t count;

} DList;

static inline DList DList_init(void)
{
    return (DList) {
        .head = NULL,
        .tail = NULL,
        .count = 0
    };
}




/* =======================================================
 *
 * str_t exclusive for this dbg only
 *
 * =======================================================
 */

typedef struct {
    char *buf;
    size_t size;
    bool isptr;
} Dstr_t;

static inline void Dpstr_free(Dstr_t *pstr)
{
    free(pstr->buf);
    if (pstr->isptr) free(pstr);
}

static Dstr_t * new_Dpstr(char *buffer) 
{
    Dstr_t *str = (Dstr_t *)malloc(sizeof(Dstr_t));
    if (str == NULL) return NULL;
    str->size = strlen(buffer);
    str->isptr = true;

    str->buf = (char *)malloc(sizeof(char) * (str->size +1));
    if (str->buf == NULL) return NULL;

    strcpy(str->buf, buffer);

    return str;
}


static inline void Dstr_print(Dstr_t *pstr)
{
    printf("%.*s",(int)(pstr->size+1),pstr->buf);
}

// ===========================================================



/*
 *
 *
 *  FUNCTION DECLARATION
 *
 *
 */


typedef void (*DList_print_func)(void*);

DNode *  DNode_init(void *value, DValType type);

bool    DList_append_node(DList *list, DNode *node);
bool    DList_delete_node(DList *list, DNode *node);
bool    DList_delete_node_by_value(DList *list, void *pointer);
void    DList_destory(DList *list);
void    DList_print(DList *list);







/*
 *
 * FUNCTION DEFINITION -
 * ------------------- |
 *                     v
 */

DNode * DNode_init(void *value, DValType type)
{

    DNode *node = (DNode *) malloc(sizeof(DNode));
    if (node == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        return NULL;
    }

    if (value == NULL) {
        fprintf(stderr, "%s: value argument is null\n", __func__);
        exit(1);
    }

    switch(type) {
        case DVT_ADDR:
            node->value = value;
            node->type = DVT_ADDR;
            break;
        case DVT_PSTR:
            node->value = new_Dpstr((char *)value);
            node->type = DVT_PSTR;
            break;
        default:
            fprintf(stderr, "%s: type not accounted for \n", __func__);
            exit(1);
    }

    node->next = node->prev = NULL;

    return node;
}


bool DList_append_node(DList *list, DNode *node)
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

void DNode_destory(DNode *del)
{
    switch(del->type) {
        case DVT_PSTR: 
            Dpstr_free((Dstr_t *)del->value);
            break;
        case DVT_ADDR:
            free(del);
            break;
        default:
            fprintf(stderr, "%s: type not accounted for\n", __func__);
            exit(1);
    }
    del = NULL;
}



bool DList_delete_node(DList *list, DNode *node)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        return false;
    }

    if (node == NULL) {
        fprintf(stderr, "%s: node argument is null\n", __func__);
        return false;
    }

    DNode *del = NULL;
    if (list->head == node) {

        del = list->head; 

        list->head = list->head->next;
        if (list->head != NULL)
            list->head->prev = NULL;

        list->count--;
        DNode_destory(del);
        return true;

    } else if (list->tail == node) {

        del = list->tail; 

        list->tail = list->tail->prev;
        if (list->tail != NULL)
            list->tail->next = NULL;

        list->count--;
        DNode_destory(del);
        return true;
    }

    DNode *tmp = list->head->next;
    while (tmp != NULL) {
        if (tmp == node) {
            del = tmp;
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
            list->count--;
            DNode_destory(del);
            return true;
        }
        tmp = tmp->next;
    }

    return false;
}



void DList_print(DList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        exit(1);
    } else if (list->head == NULL) {
        fprintf(stderr, "%s: list is empty\n", __func__);
        exit(1);
    }

    DNode *track = list->head;
    while (track != NULL) {
        switch(track->type) {
            case DVT_ADDR:
                printf("%p", track->value);
                break;
            case DVT_PSTR:
                Dstr_print((Dstr_t *)track->value);
                break;
            default:
                fprintf(stderr, "%s: type not accounted for\n", __func__);
                exit(1);
        }
        track = track->next;
    }
}

void DList_destory(DList *list)
{
    if (list == NULL) {
        fprintf(stderr ,"%s: list argument is null\n", __func__);
        exit(1);
    }
    DNode *tmp = list->head;
    DNode *del = NULL;
    while (tmp != NULL) {
        del = tmp;
        tmp = tmp->next;
        DNode_destory(del);
    }
}

bool DList_delete_node_by_value(DList *list, void *value)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        exit(1);
    } else if (value == NULL) {
        fprintf(stderr, "%s: value argument is null\n", __func__);
        exit(1);
    }

    DNode *del = NULL;
    if (list->head->value == value) {

        del = list->head; 

        list->head = list->head->next;
        if (list->head != NULL)
            list->head->prev = NULL;

        list->count--;
        DNode_destory(del);
        return true;

    } else if (list->tail->value == value) {

        del = list->tail; 

        list->tail = list->tail->prev;
        if (list->tail != NULL)
            list->tail->next = NULL;

        list->count--;
        DNode_destory(del);
        return true;
    }

    DNode *tmp = list->head->next;
    while (tmp != NULL) {
        if (tmp->value == value) {
            del = tmp;
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
            list->count--;
            DNode_destory(del);
            return true;
        }
        tmp = tmp->next;
    }

    return false;

}


#endif //_DBG_LIST_H_

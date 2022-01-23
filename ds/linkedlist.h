#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>


typedef struct node {

    void * value;
    uint64_t value_size;
    struct node *next;
    struct node *prev;

} node_t;

typedef struct {

    struct node *head;
    struct node *tail;
    size_t count;

} llist_t;

static inline llist_t llist_init(void)
{
    return (llist_t) {
        .head = NULL,
        .tail = NULL,
        .count = 0
    };
}



node_t * node_init(void *value, uint32_t value_size);

bool    llist_append_node(llist_t *list, node_t *node);
bool    llist_delete_node(llist_t *list, node_t *node);
bool    llist_delete_node_by_value(llist_t *list, void *value,  bool (*compare)(node_t *arg01, void *arg02));
void    llist_print(llist_t *list, void (*print)(void *));
void    llist_destory(llist_t *list);


/*
 *
 * FUNCTION DEFINITION -
 * ------------------- |
 *                     v
 */

node_t * node_init(void *value_addr, uint32_t value_size)
{
    node_t *node = (node_t *) malloc(sizeof(node_t));
    if (node == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        return NULL;
    }

    node->value = calloc(1, value_size);
    assert(node->value);

    memcpy(node->value, value_addr, value_size);

    node->value_size = value_size;
    node->next = node->prev = NULL;

    return node;
}
bool llist_append_node(llist_t *list, node_t *node)
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

void node_destroy(node_t *del)
{
    free(del->value);
    free(del);
    del = NULL;
}



bool llist_delete_node(llist_t *list, node_t *node)
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        return false;
    }

    if (node == NULL) {
        fprintf(stderr, "%s: node argument is null\n", __func__);
        return false;
    }

    node_t *del = NULL;
    if (list->head == node) {

        del = list->head; 

        list->head = list->head->next;
        if (list->head != NULL)
            list->head->prev = NULL;

        list->count--;
        node_destroy(del);
        return true;

    } else if (list->tail == node) {

        del = list->tail; 

        list->tail = list->tail->prev;
        if (list->tail != NULL)
            list->tail->next = NULL;
        list->count--;
        node_destroy(del);
        return true;
    }

    node_t *tmp = list->head->next;
    while (tmp != NULL) {
        if (tmp == node) {
            del = tmp;
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
            list->count--;
            node_destroy(del);
            return true;
        }
        tmp = tmp->next;
    }

    return false;
}



void llist_print(llist_t *list, void (*print)(void *))
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        exit(1);
    } else if (list->head == NULL) {
        fprintf(stderr, "%s: list is empty\n", __func__);
        exit(1);
    }

    node_t *track = list->head;
    while (track != NULL) {

        print(track->value);
        track = track->next;

    }
}

void llist_destroy(llist_t *list)
{
    if (list == NULL) {
        fprintf(stderr ,"%s: list argument is null\n", __func__);
        exit(1);
    }
    node_t *tmp = list->head;
    node_t *del = NULL;
    while (tmp != NULL) {
        del = tmp;
        tmp = tmp->next;
        node_destroy(del);
    }
}

bool llist_delete_node_by_value(llist_t *list, void *value_addr,  bool (*compare)(node_t *arg01, void *arg02))
{
    if (list == NULL) {
        fprintf(stderr, "%s: list argument is null\n", __func__);
        exit(1);
    } 
    node_t *del = NULL;
    if (compare(list->head, value_addr)) {

        del = list->head; 

        list->head = list->head->next;
        if (list->head != NULL)
            list->head->prev = NULL;

        list->count--;
        node_destroy(del);
        return true;

    } else if (compare(list->tail, value_addr)) {

        del = list->tail; 

        list->tail = list->tail->prev;
        if (list->tail != NULL)
            list->tail->next = NULL;

        list->count--;
        node_destroy(del);
        return true;
    }

    node_t *tmp = list->head->next;
    while (tmp != NULL) {
        if (compare(tmp, value_addr)) {
            del = tmp;
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
            list->count--;
            node_destroy(del);
            return true;
        }
        tmp = tmp->next;
    }

    return false;


}



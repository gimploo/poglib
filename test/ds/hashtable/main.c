#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../../../basic.h"

// Simple struct for testing
typedef struct {
    int x;
    int y;
} Point;

void print_point(void *p) {
    Point *pt = (Point *)p;
    printf("(%d, %d)", pt->x, pt->y);
}

void test_insert_and_get() {
    hashtable_t table = hashtable_init(8, Point);
    Point p = {1, 2};

    hashtable_insert(&table, myKey, p);
    Point *got = (Point *)hashtable_get_value(&table, "myKey");

    assert(got != NULL);
    assert(got->x == 1 && got->y == 2);

    hashtable_destroy(&table);
}

void test_key_existence() {
    hashtable_t table = hashtable_init(4, Point);
    Point p = {7, 8};
    hashtable_insert(&table, keyZ, p);

    assert(hashtable_is_key_used(&table, "keyZ"));
    assert(!hashtable_is_key_used(&table, "missingKey"));

    hashtable_destroy(&table);
}

void test_delete_key() {
    hashtable_t table = hashtable_init(4, Point);
    Point a = {9, 10}, b = {11, 12}, c = {13, 14};

    hashtable_insert(&table, A, a);
    hashtable_insert(&table, B, b);
    hashtable_insert(&table, C, c);

    hashtable_delete(&table, "B");

    assert(!hashtable_is_key_used(&table, "B"));
    assert(hashtable_is_key_used(&table, "A"));
    assert(hashtable_is_key_used(&table, "C"));

    hashtable_destroy(&table);
}

void test_robin_hood_probe_swap() {
    hashtable_t table = hashtable_init(4, Point);
    Point a = {1, 1}, b = {2, 2}, c = {3, 3};

    // Insert in such a way that they collide and Robin Hood triggers
    hashtable_insert(&table, A, a);
    hashtable_insert(&table, B, b);
    hashtable_insert(&table, C, c);

    assert(table.len == 3);
    hashtable_destroy(&table);
}

void test_destroy_frees_keys() {
    hashtable_t table = hashtable_init(3, Point);
    Point p = {7, 8};

    hashtable_insert(&table, test, p);
    hashtable_destroy(&table);

    // No crash = success
}

int main() {
    dbg_init();
    test_insert_and_get();
    test_key_existence();
    test_delete_key();
    test_robin_hood_probe_swap();
    test_destroy_frees_keys();

    printf("All tests passed.\n");
    dbg_destroy();
    return 0;
}


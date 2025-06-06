#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../../../basic.h"

// Simple struct for testing
typedef struct {
    int x;
    int y;
    int z;
} Point;

void print_point(Point *point)
{
    printf("%i, %i, %i\n", point->x, point->y, point->z);
}

void hashtable_test()
{
    hashtable_t table = hashtable_init(5, Point);
    Point p = { 1, 2, 3 };
    hashtable_insert(&table, "point1", (&(Point){1, 2, 3}));
    hashtable_insert(&table, "point2", &p);
    hashtable_insert(&table, "point3", &p);
    hashtable_insert(&table, "point4", &p);
    hashtable_insert(&table, "point5", &p);
    hashtable_print(&table, print_point);
    hashtable_destroy(&table);
}

void hashtable_insert_test() {
}


int main(void)
{
    dbg_init();
    hashtable_test();
    dbg_destroy();
    return 0;
}


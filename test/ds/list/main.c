#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <poglib/basic.h>

#undef TEST

static int total_tests = 0;
static int passed_tests = 0;

#define TEST(name) do { \
    total_tests++; \
    printf("  %s ... ", name); \
    fflush(stdout); \
} while(0)

#define PASS() do { \
    passed_tests++; \
    printf("PASS\n"); \
} while(0)

#define FAIL(msg) do { \
    printf("FAILED (%s)\n", msg); \
    return; \
} while(0)

void test_malloc_init(void)
{
    TEST("malloc init");
    list_t l = list_init(int, NULL);
    if (!list_is_init(&l)) FAIL("not initialized");
    if (l.arena != NULL) FAIL("arena should be NULL");
    if (l.data == NULL) FAIL("data is NULL");
    list_destroy(&l);
    PASS();
}

void test_malloc_append_read(void)
{
    TEST("malloc append and read");
    list_t l = list_init(int, NULL);
    for (int i = 0; i < 10; i++) {
        int v = i;
        list_append(&l, v);
    }
    if (l.len != 10) FAIL("wrong length");
    for (int i = 0; i < 10; i++) {
        int *v = (int *)list_get_value(&l, i);
        if (*v != i) FAIL("wrong value");
    }
    list_destroy(&l);
    PASS();
}

void test_malloc_append_ptr(void)
{
    TEST("malloc append_ptr");
    list_t l = list_init(char *, NULL);
    char *strings[] = {"hello", "world", "foo"};
    for (int i = 0; i < 3; i++)
        list_append_ptr(&l, strings[i]);
    if (l.len != 3) FAIL("wrong length");
    for (int i = 0; i < 3; i++) {
        char **vp = (char **)list_get_value(&l, i);
        if (*vp != strings[i]) FAIL("wrong pointer");
    }
    list_destroy(&l);
    PASS();
}

void test_malloc_delete(void)
{
    TEST("malloc delete");
    list_t l = list_init(int, NULL);
    for (int i = 0; i < 5; i++) {
        int v = i;
        list_append(&l, v);
    }
    list_delete(&l, 2);
    if (l.len != 4) FAIL("wrong length after delete");
    int expected[] = {0, 1, 3, 4};
    for (int i = 0; i < 4; i++) {
        int *v = (int *)list_get_value(&l, i);
        if (*v != expected[i]) FAIL("wrong value after delete");
    }
    list_destroy(&l);
    PASS();
}

void test_malloc_clear(void)
{
    TEST("malloc clear");
    list_t l = list_init(int, NULL);
    for (int i = 0; i < 5; i++) {
        int v = i;
        list_append(&l, v);
    }
    list_clear(&l);
    if (l.len != 0) FAIL("len should be 0 after clear");
    if (l.internal.top != -1) FAIL("top should be -1 after clear");
    list_destroy(&l);
    PASS();
}

void test_malloc_combine(void)
{
    TEST("malloc combine");
    list_t a = list_init(int, NULL);
    list_t b = list_init(int, NULL);
    for (int i = 0; i < 3; i++) { int v = i; list_append(&a, v); }
    for (int i = 3; i < 6; i++) { int v = i; list_append(&b, v); }
    list_combine(&a, &b);
    if (a.len != 6) FAIL("wrong length after combine");
    for (int i = 0; i < 6; i++) {
        int *v = (int *)list_get_value(&a, i);
        if (*v != i) FAIL("wrong value after combine");
    }
    list_destroy(&a);
    list_destroy(&b);
    PASS();
}

void test_arena_init(void)
{
    TEST("arena init");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(int, &arena);
    if (!list_is_init(&l)) FAIL("not initialized");
    if (l.arena != &arena) FAIL("arena pointer mismatch");
    if (l.data == NULL) FAIL("data is NULL");
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_append_read(void)
{
    TEST("arena append and read");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(int, &arena);
    for (int i = 0; i < 10; i++) {
        int v = i;
        list_append(&l, v);
    }
    if (l.len != 10) FAIL("wrong length");
    for (int i = 0; i < 10; i++) {
        int *v = (int *)list_get_value(&l, i);
        if (*v != i) FAIL("wrong value");
    }
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_grow(void)
{
    TEST("arena grow past initial capacity");
    arena_t arena = arena_init(NULL, 16384);
    list_t l = list_init(int, &arena);
    int n = 100;
    for (int i = 0; i < n; i++) {
        int v = i;
        list_append(&l, v);
    }
    if (l.len != (u64)n) FAIL("wrong length after growth");
    for (int i = 0; i < n; i++) {
        int *v = (int *)list_get_value(&l, i);
        if (*v != i) FAIL("wrong value after growth");
    }
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_delete_no_shrink(void)
{
    TEST("arena delete does not shrink capacity");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(int, &arena);
    for (int i = 0; i < 8; i++) {
        int v = i;
        list_append(&l, v);
    }
    u64 cap_before = l.internal.capacity;
    list_delete(&l, 3);
    list_delete(&l, 3);
    list_delete(&l, 3);
    if (l.internal.capacity != cap_before) FAIL("capacity should not shrink with arena");
    int expected[] = {0, 1, 2, 6, 7};
    if (l.len != 5) FAIL("wrong length after deletes");
    for (int i = 0; i < 5; i++) {
        int *v = (int *)list_get_value(&l, i);
        if (*v != expected[i]) FAIL("wrong value after deletes");
    }
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_clear(void)
{
    TEST("arena clear and refill");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(int, &arena);
    for (int i = 0; i < 10; i++) {
        int v = i;
        list_append(&l, v);
    }
    list_clear(&l);
    if (l.len != 0) FAIL("len should be 0");
    if (l.internal.top != -1) FAIL("top should be -1");
    for (int i = 0; i < 10; i++) {
        int v = i * 2;
        list_append(&l, v);
    }
    if (l.len != 10) FAIL("wrong length after re-fill");
    for (int i = 0; i < 10; i++) {
        int *v = (int *)list_get_value(&l, i);
        if (*v != i * 2) FAIL("wrong value after clear+refill");
    }
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_multiple_lists(void)
{
    TEST("arena multiple lists from same arena");
    arena_t arena = arena_init(NULL, 16384);
    list_t ints = list_init(int, &arena);
    list_t floats = list_init(float, &arena);
    for (int i = 0; i < 20; i++) {
        int vi = i;
        float vf = (float)i * 1.5f;
        list_append(&ints, vi);
        list_append(&floats, vf);
    }
    if (ints.len != 20) FAIL("wrong ints length");
    if (floats.len != 20) FAIL("wrong floats length");
    for (int i = 0; i < 20; i++) {
        int *iv = (int *)list_get_value(&ints, i);
        float *fv = (float *)list_get_value(&floats, i);
        if (*iv != i) FAIL("wrong int value");
        if (*fv != (float)i * 1.5f) FAIL("wrong float value");
    }
    list_destroy(&ints);
    list_destroy(&floats);
    arena_destroy(&arena);
    PASS();
}

void test_arena_pointer_type(void)
{
    TEST("arena pointer type list");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(char *, &arena);
    char *words[] = {"alpha", "beta", "gamma", "delta"};
    for (int i = 0; i < 4; i++)
        list_append_ptr(&l, words[i]);
    if (l.len != 4) FAIL("wrong length");
    for (int i = 0; i < 4; i++) {
        char **vp = (char **)list_get_value(&l, i);
        if (*vp != words[i]) FAIL("wrong pointer value");
    }
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_struct_type(void)
{
    TEST("arena struct type");
    typedef struct {
        int x, y, z;
        char label[16];
    } point_t;
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(point_t, &arena);
    for (int i = 0; i < 10; i++) {
        point_t p = {i, i*2, i*3, "pt"};
        list_append(&l, p);
    }
    if (l.len != 10) FAIL("wrong length");
    for (int i = 0; i < 10; i++) {
        point_t *p = (point_t *)list_get_value(&l, i);
        if (p->x != i || p->y != i*2 || p->z != i*3) FAIL("wrong struct field");
    }
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_append_multiple(void)
{
    TEST("arena append_multiple");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(int, &arena);
    int data[] = {10, 20, 30, 40, 50};
    list_append_multiple(&l, data);
    if (l.len != 5) FAIL("wrong length");
    for (int i = 0; i < 5; i++) {
        int *v = (int *)list_get_value(&l, i);
        if (*v != data[i]) FAIL("wrong value");
    }
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_destroy_empty(void)
{
    TEST("arena destroy empty list");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(int, &arena);
    list_destroy(&l);
    PASS();
    arena_destroy(&arena);
}

void test_malloc_destroy_empty(void)
{
    TEST("malloc destroy empty list");
    list_t l = list_init(int, NULL);
    list_destroy(&l);
    PASS();
}

void test_arena_data_within_arena(void)
{
    TEST("arena data lives in arena memory");
    arena_t arena = arena_init(NULL, 4096);
    list_t l = list_init(int, &arena);
    u8 *arena_start = arena.memory;
    u8 *arena_end = arena.memory + arena.capacity;
    for (int i = 0; i < 10; i++) {
        int v = i;
        list_append(&l, v);
    }
    if (l.data < arena_start || l.data >= arena_end)
        FAIL("list data is not within arena memory");
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

void test_arena_growth_uses_arena(void)
{
    TEST("arena growth allocations are in arena");
    arena_t arena = arena_init(NULL, 16384);
    list_t l = list_init(int, &arena);
    u8 *arena_start = arena.memory;
    u8 *arena_end = arena.memory + arena.capacity;
    for (int i = 0; i < 500; i++) {
        int v = i;
        list_append(&l, v);
    }
    if (l.data < arena_start || l.data >= arena_end)
        FAIL("grown data is not within arena memory");
    list_destroy(&l);
    arena_destroy(&arena);
    PASS();
}

int main(void)
{
    printf("========================================\n");
    printf("   LIST ARENA INTEGRATION TESTS\n");
    printf("========================================\n\n");
    printf("--- malloc mode (backward compat) ---\n");
    test_malloc_init();
    test_malloc_append_read();
    test_malloc_append_ptr();
    test_malloc_delete();
    test_malloc_clear();
    test_malloc_combine();
    test_malloc_destroy_empty();

    printf("\n--- arena mode ---\n");
    test_arena_init();
    test_arena_append_read();
    test_arena_grow();
    test_arena_delete_no_shrink();
    test_arena_clear();
    test_arena_multiple_lists();
    test_arena_pointer_type();
    test_arena_struct_type();
    test_arena_append_multiple();
    test_arena_destroy_empty();
    test_arena_data_within_arena();
    test_arena_growth_uses_arena();

    printf("\n========================================\n");
    printf("  %d / %d tests passed\n", passed_tests, total_tests);
    printf("========================================\n");
    return passed_tests == total_tests ? 0 : 1;
}

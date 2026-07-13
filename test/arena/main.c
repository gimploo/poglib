#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <threads.h>
#include "../../basic/arena.h"

#define ARENA_CAPACITY 4096
#define TEST_PATTERN_1 0xAB
#define TEST_PATTERN_2 0xCD

/* ── Single-threaded tests ───────────────────────────────────────────── */

static void test_init_null(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    assert(arena->memory != NULL);
    assert(arena->capacity == ARENA_CAPACITY);
    assert(arena->size > 0);  /* metadata stored in buffer */
    assert(arena->meta.lifetime_owner == NULL);
    arena_destroy(arena);
    printf("PASS: test_init_null\n");
}

static void test_init_sub_arena(void) {
    arena_t *parent = arena_init(NULL, ARENA_CAPACITY);
    arena_t *sub = arena_init(parent, 256);
    assert(sub->memory != NULL);
    assert(sub->capacity == 256);
    assert(sub->meta.lifetime_owner == parent);
    assert((u8 *)sub->memory >= parent->memory);
    assert((u8 *)sub->memory + 256 <= parent->memory + parent->capacity);
    arena_destroy(sub);
    arena_destroy(parent);
    printf("PASS: test_init_sub_arena\n");
}

static void test_reserve_basic(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    void *p1 = arena_reserve(arena, 64);
    void *p2 = arena_reserve(arena, 64);
    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p1 != p2);
    assert((u8 *)p2 >= (u8 *)p1 + 64);
    arena_destroy(arena);
    printf("PASS: test_reserve_basic\n");
}

static void test_reserve_alignment(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    for (int i = 0; i < 20; i++) {
        void *p = arena_reserve(arena, 1 + (i % 7));
        assert(p != NULL);
        assert(((u64)p & 0xF) == 0);
    }
    arena_destroy(arena);
    printf("PASS: test_reserve_alignment\n");
}

static void test_giveback_reuse(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    void *p1 = arena_reserve(arena, 100);
    assert(p1 != NULL);
    arena_giveback(arena, p1, 100);
    void *p2 = arena_reserve(arena, 100);
    assert(p2 == p1);
    arena_destroy(arena);
    printf("PASS: test_giveback_reuse\n");
}

static void test_store(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    const char test_str[] = "Hello, Arena!";
    size_t len = strlen(test_str) + 1;
    char *stored = (char *)arena_store(arena, test_str, len);
    assert(stored != NULL);
    assert(stored != test_str);
    assert(memcmp(stored, test_str, len) == 0);
    arena_destroy(arena);
    printf("PASS: test_store\n");
}

static void test_clear(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    arena_reserve(arena, 50);
    arena_clear(arena);
    void *p2 = arena_reserve(arena, 50);
    assert(p2 >= (void *)(arena->memory + sizeof(arena_t)));  /* metadata preserved */
    arena_destroy(arena);
    printf("PASS: test_clear\n");
}

static void test_is_init(void) {
    arena_t zeroed = {0};
    assert(arena_is_init(&zeroed) == false);
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    assert(arena_is_init(arena) == true);
    arena_destroy(arena);
    printf("PASS: test_is_init\n");
}

static void test_destroy_top_level_only(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    arena_destroy(arena);
    printf("PASS: test_destroy_top_level_only\n");
}

static void test_destroy_sub_arena(void) {
    arena_t *parent = arena_init(NULL, ARENA_CAPACITY);
    for (int i = 0; i < 5; i++) {
        arena_t *sub = arena_init(parent, sizeof(arena_t) + 32);  /* enough for metadata + allocation */
        void *p = arena_reserve(sub, 16);
        assert(p != NULL);
        arena_destroy(sub);
    }
    arena_destroy(parent);
    printf("PASS: test_destroy_sub_arena\n");
}

static void test_many_small_alloc(void) {
    arena_t *arena = arena_init(NULL, ARENA_CAPACITY);
    enum { COUNT = 50 };
    void *ptrs[COUNT];
    int sizes[COUNT];

    for (int i = 0; i < COUNT; i++) {
        sizes[i] = 4 + (i % 8) * 4;
        ptrs[i] = arena_reserve(arena, sizes[i]);
        assert(ptrs[i] != NULL);
        memset(ptrs[i], TEST_PATTERN_1, sizes[i]);
    }
    for (int i = 0; i < COUNT; i++) {
        u8 *data = (u8 *)ptrs[i];
        for (int j = 0; j < sizes[i]; j++)
            assert(data[j] == TEST_PATTERN_1);
    }
    for (int i = 0; i < COUNT; i += 2)
        arena_giveback(arena, ptrs[i], sizes[i]);
    for (int i = 0; i < COUNT; i += 2) {
        void *re = arena_reserve(arena, sizes[i]);
        assert(re != NULL);
        memset(re, TEST_PATTERN_2, sizes[i]);
    }
    arena_destroy(arena);
    printf("PASS: test_many_small_alloc\n");
}

/* ── Multi-threaded helpers ──────────────────────────────────────────── */

typedef struct {
    arena_t *arena;
    int thread_id;
    int num_ops;
    bool success;
} mt_worker_t;

static int mt_reserve_worker(void *arg) {
    mt_worker_t *w = (mt_worker_t *)arg;
    for (int i = 0; i < w->num_ops; i++) {
        void *p = arena_reserve(w->arena, 16);
        if (!p) { w->success = false; return 1; }
        u32 pattern = ((u32)w->thread_id << 16) | (u32)i;
        memcpy(p, &pattern, sizeof(pattern));
        thrd_yield();
    }
    w->success = true;
    return 0;
}

static void test_mt_parallel_reserve(void) {
    arena_t *arena = arena_init(NULL, 4096);
    thrd_t threads[4];
    mt_worker_t workers[4];

    for (int i = 0; i < 4; i++) {
        workers[i] = (mt_worker_t){
            .arena = arena,
            .thread_id = i,
            .num_ops = 20,
        };
        thrd_create(&threads[i], mt_reserve_worker, &workers[i]);
    }
    for (int i = 0; i < 4; i++) {
        thrd_join(threads[i], NULL);
        assert(workers[i].success);
    }
    arena_destroy(arena);
    printf("PASS: test_mt_parallel_reserve\n");
}

typedef struct {
    arena_t *arena;
    void **ptrs;
    int count;
    bool success;
} mt_giveback_worker_t;

static int mt_giveback_worker(void *arg) {
    mt_giveback_worker_t *w = (mt_giveback_worker_t *)arg;
    for (int i = 0; i < w->count; i++) {
        if (w->ptrs[i]) {
            arena_giveback(w->arena, w->ptrs[i], 16);
            w->ptrs[i] = NULL;
        }
        thrd_yield();
    }
    w->success = true;
    return 0;
}

static void test_mt_parallel_giveback(void) {
    arena_t *arena = arena_init(NULL, 2048);
    enum { N = 4, OPS = 10 };
    void *all_ptrs[N * OPS];

    for (int i = 0; i < N * OPS; i++) {
        all_ptrs[i] = arena_reserve(arena, 16);
        assert(all_ptrs[i] != NULL);
    }

    thrd_t threads[N];
    mt_giveback_worker_t workers[N];

    for (int i = 0; i < N; i++) {
        workers[i] = (mt_giveback_worker_t){
            .arena = arena,
            .ptrs = &all_ptrs[i * OPS],
            .count = OPS,
        };
        thrd_create(&threads[i], mt_giveback_worker, &workers[i]);
    }
    for (int i = 0; i < N; i++) {
        thrd_join(threads[i], NULL);
        assert(workers[i].success);
    }
    arena_destroy(arena);
    printf("PASS: test_mt_parallel_giveback\n");
}

static int mt_mixed_worker(void *arg) {
    mt_worker_t *w = (mt_worker_t *)arg;
    for (int i = 0; i < w->num_ops; i++) {
        void *p = arena_reserve(w->arena, 8);
        if (p) {
            thrd_yield();
            arena_giveback(w->arena, p, 8);
        }
    }
    w->success = true;
    return 0;
}

static void test_mt_mixed_stress(void) {
    arena_t *arena = arena_init(NULL, 4096);
    thrd_t threads[8];
    mt_worker_t workers[8];

    for (int i = 0; i < 8; i++) {
        workers[i] = (mt_worker_t){
            .arena = arena,
            .thread_id = i,
            .num_ops = 100,
        };
        thrd_create(&threads[i], mt_mixed_worker, &workers[i]);
    }
    for (int i = 0; i < 8; i++) {
        thrd_join(threads[i], NULL);
        assert(workers[i].success);
    }
    arena_destroy(arena);
    printf("PASS: test_mt_mixed_stress\n");
}

typedef struct {
    arena_t *parent;
    bool success;
} mt_sub_arena_worker_t;

static int mt_sub_arena_worker(void *arg) {
    mt_sub_arena_worker_t *w = (mt_sub_arena_worker_t *)arg;
    arena_t *sub = arena_init(w->parent, 128);
    if (!arena_is_init(sub)) {
        w->success = false; return 1;
    }
    void *p = arena_reserve(sub, 32);
    if (!p) {
        w->success = false; return 1;
    }
    thrd_yield();
    arena_destroy(sub);
    w->success = true;
    return 0;
}

static void test_mt_sub_arenas(void) {
    arena_t *parent = arena_init(NULL, ARENA_CAPACITY);
    thrd_t threads[4];
    mt_sub_arena_worker_t workers[4];

    for (int i = 0; i < 4; i++) {
        workers[i] = (mt_sub_arena_worker_t){
            .parent = parent,
        };
        thrd_create(&threads[i], mt_sub_arena_worker, &workers[i]);
    }
    for (int i = 0; i < 4; i++) {
        thrd_join(threads[i], NULL);
        assert(workers[i].success);
    }
    arena_destroy(parent);
    printf("PASS: test_mt_sub_arenas\n");
}

static int mt_store_worker(void *arg) {
    mt_worker_t *w = (mt_worker_t *)arg;
    for (int i = 0; i < w->num_ops; i++) {
        u64 data = ((u64)w->thread_id << 32) | (u64)i;
        u64 *stored = (u64 *)arena_store(w->arena, &data, sizeof(data));
        if (!stored) { w->success = false; return 1; }
        assert(*stored == data);
        thrd_yield();
    }
    w->success = true;
    return 0;
}

static void test_mt_arena_store(void) {
    arena_t *arena = arena_init(NULL, 4096);
    thrd_t threads[4];
    mt_worker_t workers[4];

    for (int i = 0; i < 4; i++) {
        workers[i] = (mt_worker_t){
            .arena = arena,
            .thread_id = i,
            .num_ops = 20,
        };
        thrd_create(&threads[i], mt_store_worker, &workers[i]);
    }
    for (int i = 0; i < 4; i++) {
        thrd_join(threads[i], NULL);
        assert(workers[i].success);
    }
    arena_destroy(arena);
    printf("PASS: test_mt_arena_store\n");
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void) {
    /* Single-threaded */
    test_init_null();
    test_init_sub_arena();
    test_reserve_basic();
    test_reserve_alignment();
    test_giveback_reuse();
    test_store();
    test_clear();
    test_is_init();
    test_destroy_top_level_only();
    test_destroy_sub_arena();
    test_many_small_alloc();

    /* Multi-threaded */
    test_mt_parallel_reserve();
    test_mt_parallel_giveback();
    test_mt_mixed_stress();
    test_mt_sub_arenas();
    test_mt_arena_store();

    printf("\nAll arena tests passed!\n");
    return 0;
}

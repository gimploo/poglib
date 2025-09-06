#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <assert.h>
#include "../../basic/arena.h"  // Your arena header

#define ARENA_CAPACITY 1024

// Simple test: Reserve memory and give it back, then re-reserve same size should reuse
void test_single_thread_basic() {
    arena_t arena = arena_init(NULL, ARENA_CAPACITY);

    void *p1 = arena_reserve(&arena, 100);
    assert(p1 != NULL);

    arena_giveback(&arena, p1, 100);

    void *p2 = arena_reserve(&arena, 100);
    assert(p2 == p1);  // Should reuse same chunk from freelist

    arena_destroy(&arena);
    printf("test_single_thread_basic passed\n");
}

// Test that arena_clear resets size and subsequent allocations start fresh
void test_clear() {
    arena_t arena = arena_init(NULL, ARENA_CAPACITY);

    void *p1 = arena_reserve(&arena, 50);
    arena_clear(&arena);

    void *p2 = arena_reserve(&arena, 50);
    assert(p2 == arena.memory); // After clear, allocations start from beginning

    arena_destroy(&arena);
    printf("test_clear passed\n");
}

// Multithreaded test: multiple threads allocating and giving back concurrently
int thread_func(void *arg) {
    arena_t *arena = (arena_t *)arg;
    for (int i = 0; i < 100; i++) {
        void *p = arena_reserve(arena, 10);
        thrd_yield(); // Yield to increase contention chances
        arena_giveback(arena, p, 10);
    }
    return 0;
}

void test_multithreaded() {
    arena_t arena = arena_init(NULL, ARENA_CAPACITY);

    thrd_t threads[4];
    for (int i = 0; i < 4; i++) {
        thrd_create(&threads[i], thread_func, &arena);
    }
    for (int i = 0; i < 4; i++) {
        thrd_join(threads[i], NULL);
    }

    arena_destroy(&arena);
    printf("test_multithreaded passed\n");
}

int main() {
    test_single_thread_basic();
    test_clear();
    test_multithreaded();

    printf("All tests passed!\n");
    return 0;
}


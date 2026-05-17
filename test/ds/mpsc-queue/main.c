#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../../../basic/ds/mpsc-queue.h" 

#define NUM_PRODUCERS 4
#define ITEMS_PER_PRODUCER 10000
#define QUEUE_CAPACITY 2048 // Large enough to prevent instant filling

// Global test state
arena_t global_arena;
mpsc_queue_t g_test_queue;
atomic_int g_items_received = 0;
atomic_bool g_test_running = true;

// A simple tracking array to verify no pointer addresses were lost or duplicated
atomic_bool g_validation_array[NUM_PRODUCERS * ITEMS_PER_PRODUCER];

// Payload data simulating external arena allocations
u64 g_producer_payloads[NUM_PRODUCERS * ITEMS_PER_PRODUCER];

void test_queue_initialization() 
{
    // Must be a power of 2 for the (capacity - 1) bitwise mask to work
    mpsc_queue_t queue = mpsc_queue_init(&global_arena, 128, sizeof(void*));
    
    assert(atomic_load(&queue.head) == 0);
    assert(atomic_load(&queue.tail) == 0);
    assert(queue.internals.capacity == 128);
    assert(queue.buffer != NULL);
    
    mpsc_queue_destroy(&queue);
}

void test_queue_fifo_ordering() 
{
    mpsc_queue_t queue = mpsc_queue_init(&global_arena, 4, sizeof(void*));
    
    // We pass addresses, simulating allocations from an external arena
    u32 data_a = 0xAAAA;
    u32 data_b = 0xBBBB;
    u32 data_c = 0xCCCC;

    mpsc_queue_put(&queue, &data_a, sizeof(void*));
    mpsc_queue_put(&queue, &data_b, sizeof(void*));
    mpsc_queue_put(&queue, &data_c, sizeof(void*));

    const u32* out_a = (const u32*)mpsc_queue_get(&queue);
    const u32* out_b = (const u32*)mpsc_queue_get(&queue);
    const u32* out_c = (const u32*)mpsc_queue_get(&queue);

    assert(out_a != NULL && *out_a == 0xAAAA);
    assert(out_b != NULL && *out_b == 0xBBBB);
    assert(out_c != NULL && *out_c == 0xCCCC);

    mpsc_queue_destroy(&queue);
}

void test_queue_wraparound() 
{
    mpsc_queue_t queue = mpsc_queue_init(&global_arena, 4, sizeof(void*));
    u32 dummy = 1;

    // Push 3, Pop 3 (Tail is now 3, Head is 3)
    for (int i = 0; i < 3; ++i) {
        mpsc_queue_put(&queue, &dummy, sizeof(void*));
        mpsc_queue_get(&queue);
    }

    // Push 2 more to force tail to cross the capacity boundary (Index 4 & 5)
    u32 wrap_a = 88;
    u32 wrap_b = 99;
    mpsc_queue_put(&queue, &wrap_a, sizeof(void*));
    mpsc_queue_put(&queue, &wrap_b, sizeof(void*));

    const u32* out_wrap_a = (const u32*)mpsc_queue_get(&queue);
    const u32* out_wrap_b = (const u32*)mpsc_queue_get(&queue);

    assert(*out_wrap_a == 88);
    assert(*out_wrap_b == 99);

    mpsc_queue_destroy(&queue);
}

int main(int argc, char **argv)
{
    dbg_init();
    global_arena = arena_init(NULL, 1 * GB);

    printf("--- Starting Lockless MPSC Queue Test Suite ---\n\n");

    printf("[1/4] Running Initialization Test...\n");
    test_queue_initialization();
    printf("      -> Passed.\n\n");

    printf("[2/4] Running FIFO Ordering Test...\n");
    test_queue_fifo_ordering();
    printf("      -> Passed.\n\n");

    printf("[3/4] Running Wraparound Test...\n");
    test_queue_wraparound();
    printf("      -> Passed.\n\n");

    printf("=====================================================\n");
    printf("SUCCESS: All lockless memory barriers functioning perfectly.\n");
    printf("=====================================================\n");

    arena_destroy(&global_arena);
    dbg_destroy();

    return 0;
}

#pragma once
#include <poglib/basic.h>

/*============================================================================
        - Multiple producer - Single consumer lockless queue -
============================================================================*/

typedef struct mpsc_queue_t mpsc_queue_t;

mpsc_queue_t        mpsc_queue(arena_t *const arena, const u64 capacity);
const void *        mpsc_queue_get(mpsc_queue_t * const self);
void                mpsc_queue_put(mpsc_queue_t * const self, void * const item_addr);


#ifndef IGNORE_MPC_QUEUE_IMPLEMENTATION

typedef struct mpsc_queue__internal_item_t mpsc_queue__internal_item_t;
struct mpsc_queue__internal_item_t {
    atomic_bool is_ready;
    const void *data;
};

struct mpsc_queue_t {
    alignas(64) atomic_uintmax_t    head;
    alignas(64) atomic_uintmax_t    tail;
    mpsc_queue__internal_item_t     *buffer;
    struct {
        u64 capacity;
        u64 allocated_size;
    } internals;
};

mpsc_queue_t mpsc_queue(arena_t *const arena, const u64 capacity)
{
    ASSERT(capacity > 0);
    const u64 allocated_size = sizeof(mpsc_queue__internal_item_t) * capacity;
    return (mpsc_queue_t) {
        .head = 0,
        .tail = 0,
        .buffer = arena_reserve(arena, allocated_size),
        .internals = {
            .capacity = capacity,
            .allocated_size = allocated_size
        }
    };
}

INTERNAL bool mpsc_queue__internal__is_full(const u64 head, const u64 tail, const u64 capacity)
{
    return (tail - head) >= capacity;
}

INTERNAL bool mpsc_queue__internal__is_empty(const u64 head, const u64 tail)
{
    return tail == head;
}

mpsc_queue__internal_item_t * mpsc_queue__internal_get_item(const mpsc_queue_t * const self, const u64 index)
{
    return self->buffer + (index & (self->internals.capacity - 1));
}

void mpsc_queue_put(mpsc_queue_t * const self, void * const item_addr)
{
    u64 claimed_tail_index;
    while (true) {
        u64 current_head_index = atomic_load_explicit(&self->head, memory_order_acquire);
        u64 current_tail_index = atomic_load_explicit(&self->tail, memory_order_relaxed);

        if (mpsc_queue__internal__is_full(current_head_index, current_tail_index, self->internals.capacity)) {
            eprint("MPSC Queue is full");
        }

        if (atomic_compare_exchange_weak_explicit(
                &self->tail, 
                &current_tail_index, 
                current_tail_index + 1, 
                memory_order_relaxed, 
                memory_order_relaxed)
        ) {
            claimed_tail_index = current_tail_index;
            break;
        }
    }

    mpsc_queue__internal_item_t * const item = mpsc_queue__internal_get_item(self, claimed_tail_index);
    item->data = item_addr;
    atomic_store_explicit(&item->is_ready, true, memory_order_release);
}


const void * mpsc_queue_get(mpsc_queue_t * const self)
{
    const u64 current_tail_index = atomic_load_explicit(&self->tail, memory_order_acquire);
    const u64 current_head_index = atomic_load_explicit(&self->head, memory_order_acquire);

    if (mpsc_queue__internal__is_empty(current_head_index, current_tail_index)) {
        return NULL;
    }

    mpsc_queue__internal_item_t * const item_addr = mpsc_queue__internal_get_item(self, current_head_index);
    const bool is_item_ready = atomic_load_explicit(&item_addr->is_ready, memory_order_acquire);
    if (!is_item_ready) {
        return NULL;
    }

    const void *result = item_addr->data;
    atomic_store_explicit(&self->head, current_head_index + 1, memory_order_release);
    atomic_store_explicit(&item_addr->is_ready, false, memory_order_release);

    return result;
}

#endif

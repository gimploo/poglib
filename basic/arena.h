#pragma once
#include "dbg.h"
#include "common.h"
#include <stdatomic.h>
#include <threads.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/*================================================================================
 *                      -- ARENA MEMORY ALLOCATOR --
================================================================================*/

//TODO: have arena_init return a ptr

//NOTE: supports only 16 byte alignment

typedef struct arena_t arena_t;
typedef struct free_chunks_t free_chunks_t;
typedef struct arena_alloc_record_t arena_alloc_record_t;

struct arena_alloc_record_t {
    const char *file;
    int         line;
    const char *func;
    u64         size;
    arena_alloc_record_t *next;
};

struct arena_t {
    u8 *memory;
    u64 capacity;
    u64 size;
    struct {
        free_chunks_t *head;
        u32 count;
    } freelist;
    struct {
        atomic_flag lock;
        struct arena_t *lifetime_owner;
    } meta;
    struct {
        arena_alloc_record_t *head;
        u32 count;
    } alloc_log;
    const char *init_file;
    int         init_line;
    arena_t    *next_in_registry;
};

struct free_chunks_t {
    void *memory;
    free_chunks_t *next;
    u64 size;
};

arena_t     arena__internal_init(arena_t *const, const u64 capacity, const char *file, int line);
void *      arena__internal_reserve_memory_16byte_aligned(arena_t * const self, const u64 memory_size);
void *      arena__internal_reserve_tracked(arena_t *const self, const u64 memory_size, const char *file, int line, const char *func);
void *      arena_store(arena_t *const self, const void *const mem, const u64 mem_size);
bool        arena_is_init(const arena_t *const self);
void        arena_giveback(arena_t *const self, void *const ptr, const u64 size);
void        arena_clear(arena_t *const self);
void        arena_destroy(arena_t *const self);
void        arena_dump_json(const char *filepath);

#define arena_init(arena, capacity) \
    arena__internal_init((arena), (capacity), __FILE__, __LINE__)

#define arena_reserve(self, memory_size) \
    arena__internal_reserve_tracked((self), (memory_size), __FILE__, __LINE__, __func__)

#ifndef IGNORE_ARENA_IMPLEMENTATION

static arena_t *arena_registry = NULL;

static void arena_registry_add(arena_t *a)
{
    a->next_in_registry = arena_registry;
    arena_registry = a;
}

static void arena_registry_remove(arena_t *a)
{
    if (arena_registry == a) {
        arena_registry = a->next_in_registry;
        a->next_in_registry = NULL;
        return;
    }
    for (arena_t *cur = arena_registry; cur; cur = cur->next_in_registry) {
        if (cur->next_in_registry == a) {
            cur->next_in_registry = a->next_in_registry;
            a->next_in_registry = NULL;
            return;
        }
    }
}

static void arena__internal_ensure_registered(arena_t *self)
{
    if (self->meta.lifetime_owner) return;
    if (!self->init_file) return;

    for (arena_t *cur = arena_registry; cur; cur = cur->next_in_registry) {
        if (cur == self) return;
    }
    arena_registry_add(self);
}

static void arena__internal_log_alloc(arena_t *const self, u64 size, const char *file, int line, const char *func)
{
    arena_alloc_record_t *rec = calloc(1, sizeof(*rec));
    rec->file = file;
    rec->line = line;
    rec->func = func;
    rec->size = size;
    rec->next = self->alloc_log.head;
    self->alloc_log.head = rec;
    self->alloc_log.count++;
}

static void arena__internal_free_alloc_log(arena_t *const self)
{
    arena_alloc_record_t *cur = self->alloc_log.head;
    while (cur) {
        arena_alloc_record_t *next = cur->next;
        free(cur);
        cur = next;
    }
    self->alloc_log.head = NULL;
    self->alloc_log.count = 0;
}

void * arena__internal_check_in_freelist(arena_t *const self, const u64 memory_size)
{
    if (!self->freelist.count) {
        return NULL;
    }

    free_chunks_t *chunk        = self->freelist.head;
    free_chunks_t *prev_chunk   = NULL;
    void *memory                = NULL;

    for(u32 i = 0; i < self->freelist.count; i++) 
    {
        ASSERT(self->freelist.head);
        ASSERT(chunk);

        if (memory_size > chunk->size) {
            prev_chunk = chunk;
            chunk = chunk->next;
            continue;
        }

        memory = chunk->memory;

        if (prev_chunk) {
            prev_chunk->next = chunk->next;
        } else {
            self->freelist.head = chunk->next;
        }

        self->freelist.count--;
        if (self->meta.lifetime_owner) {
            arena_giveback(self->meta.lifetime_owner, chunk, sizeof(free_chunks_t));
        } else {
            free(chunk);
        }
        return memory;
    }
    return memory;
}

void * arena__internal_reserve_memory_16byte_aligned(arena_t * const self, const u64 memory_size)
{
    if (!self->freelist.count && (self->capacity - self->size) < memory_size) {
        eprint("Ran out of memory, asking for `%li`bytes but `%li`bytes only available\n\tcapacity = %li | size = %li", memory_size, (self->capacity - self->size), self->capacity, self->size);
    }
    const u64 align = 16;
    void *mem = NULL;

    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
    {
        const u64 current_ptr = (u64)self->memory + self->size;
        const u64 aligned_ptr = (current_ptr + (align - 1)) & ~(align - 1);
        const u64 padding = (u64)(aligned_ptr - current_ptr);
        const u64 offset = self->size + padding;

        if ((offset + memory_size) > self->capacity)
        {
            eprint("Arena is full");
            atomic_flag_clear(&self->meta.lock);
            return NULL;
        }

        void *res_memory = arena__internal_check_in_freelist(self, memory_size);
        if (res_memory) {
            memset(res_memory, 0, memory_size);
            atomic_flag_clear(&self->meta.lock);
            return res_memory;
        }

        mem = (void *)aligned_ptr;
        memset(mem, 0, memory_size);

        self->size = offset + memory_size;
    }
    atomic_flag_clear(&self->meta.lock);

    ASSERT(mem);
    if(((u64)(mem) & (16 - 1)) != 0) {
        eprint("Memory is not 16 byte aligned");
    };
    return mem;
}

void * arena__internal_reserve_tracked(arena_t *const self, const u64 memory_size, const char *file, int line, const char *func)
{
    arena__internal_ensure_registered(self);
    void *mem = arena__internal_reserve_memory_16byte_aligned(self, memory_size);
    if (mem) {
        arena__internal_log_alloc(self, memory_size, file, line, func);
    }
    return mem;
}

void * arena_store(arena_t * const self, const void * const mem, const u64 mem_size)
{
    ASSERT(mem_size > 0);
    ASSERT(mem);
    void *raw_mem = arena_reserve(self, mem_size);
    memcpy(raw_mem, mem, mem_size);
    return raw_mem;
}

arena_t arena__internal_init(arena_t *const arena, const u64 capacity, const char *file, int line)
{
    arena_t o = {
        .capacity = capacity,
        .size = 0,
        .memory = arena ? arena__internal_reserve_memory_16byte_aligned(arena, capacity) : calloc(capacity, sizeof(u8)),
        .freelist = {0},
        .meta = {
            .lifetime_owner = arena,
        },
        .alloc_log = {0},
        .init_file = file,
        .init_line = line,
        .next_in_registry = NULL,
    };
    atomic_flag_clear(&o.meta.lock);
    return o;
}

void arena_giveback(arena_t *const self, void *const ptr, const u64 size)
{
    ASSERT(self);
    ASSERT(ptr);
    ASSERT(size > 0);
    ASSERT(arena_is_init(self));

    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
    {
        free_chunks_t *chunk = self->freelist.head;
        while(chunk != NULL && chunk->next != NULL) {
            chunk = chunk->next;
        }

        free_chunks_t *new_chunk;
        if (self->meta.lifetime_owner) {
            new_chunk = (free_chunks_t *)arena__internal_reserve_memory_16byte_aligned(
                self->meta.lifetime_owner, sizeof(free_chunks_t));
            if (new_chunk) {
                new_chunk->memory = ptr;
                new_chunk->size = size;
                new_chunk->next = NULL;
            }
        } else {
            new_chunk = calloc(1, sizeof(free_chunks_t));
        }
        ASSERT(new_chunk);

        if (chunk) {
            chunk->next = new_chunk;
        } else {
            self->freelist.head = new_chunk;
        }
        self->freelist.count++;
    }
    atomic_flag_clear(&self->meta.lock);
}

void arena_clear(arena_t *self)
{
    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
        self->size = 0;
    atomic_flag_clear(&self->meta.lock);
    arena__internal_free_alloc_log(self);
}

void arena_destroy(arena_t *const self)
{
    if (!self->meta.lifetime_owner) {
        arena_registry_remove(self);
    }

    if (self->meta.lifetime_owner) {
        arena_giveback(self->meta.lifetime_owner, self->memory, self->capacity);
        return;
    }

    while (atomic_flag_test_and_set(&self->meta.lock)) { thrd_yield(); }
    {
        free_chunks_t *cur = self->freelist.head;
        while(cur != NULL) {
            free_chunks_t *chunk = cur;
            cur = cur->next;
            free(chunk);
        }
    }
    atomic_flag_clear(&self->meta.lock);
    arena__internal_free_alloc_log(self);
    free(self->memory);
}


bool arena_is_init(const arena_t * const self)
{
    return self->memory != NULL || self->capacity > 0;
}

void arena_dump_json(const char *filepath)
{
    FILE *f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "[arena] Failed to open '%s' for writing\n", filepath);
        return;
    }

    fprintf(f, "{\n  \"generated_at\": %lu,\n  \"arenas\": [\n", (unsigned long)time(NULL));

    u32 arena_idx = 0;
    for (arena_t *a = arena_registry; a; a = a->next_in_registry) {
        if (arena_idx > 0) fprintf(f, ",\n");
        fprintf(f, "    {\n");
        fprintf(f, "      \"init_at\": \"%s:%d\",\n", a->init_file ? a->init_file : "unknown", a->init_line);
        fprintf(f, "      \"capacity\": %lu,\n", (unsigned long)a->capacity);
        fprintf(f, "      \"used\": %lu,\n", (unsigned long)a->size);
        fprintf(f, "      \"usage_pct\": %.1f,\n", a->capacity ? 100.0 * (double)a->size / (double)a->capacity : 0.0);
        fprintf(f, "      \"free_chunks\": %u,\n", a->freelist.count);
        fprintf(f, "      \"alloc_count\": %u,\n", a->alloc_log.count);
        fprintf(f, "      \"allocations\": [\n");

        u32 alloc_idx = 0;
        for (arena_alloc_record_t *r = a->alloc_log.head; r; r = r->next) {
            if (alloc_idx > 0) fprintf(f, ",\n");
            fprintf(f, "        {");
            fprintf(f, "\"file\": \"%s\", ", r->file);
            fprintf(f, "\"line\": %d, ", r->line);
            fprintf(f, "\"func\": \"%s\", ", r->func);
            fprintf(f, "\"bytes\": %lu", (unsigned long)r->size);
            fprintf(f, "}");
            alloc_idx++;
        }

        fprintf(f, "\n      ]\n    }");
        arena_idx++;
    }

    fprintf(f, "\n  ]\n}\n");
    fclose(f);
}

#endif

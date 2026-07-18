#pragma once
#include "dbg.h"
#include "common.h"

/*================================================================================
 *                      -- ARENA MEMORY ALLOCATOR --
 ================================================================================*/

//NOTE: supports only 16 byte alignment

typedef struct arena_t arena_t;
typedef struct free_chunks_t free_chunks_t;

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
};

struct free_chunks_t {
    void *memory;
    free_chunks_t *next;
    u64 size;
};

arena_t *   arena_init(arena_t *const, const u64 capacity);

#define arena_reserve(self, memory_size) \
    arena__internal__reserve_tracked((self), (memory_size), __FILE__, __LINE__, __func__)

void *      arena_store(arena_t *const self, const void *const mem, const u64 mem_size);
bool        arena_is_init(const arena_t *const self);
void        arena__internal_giveback(arena_t *const self, void *const ptr, const u64 size);

#define arena_giveback(self, ptr, size) \
    arena__internal__giveback_tracked((self), (ptr), (size), __FILE__, __LINE__, __func__)

void        arena_clear(arena_t *const self);
void        arena_destroy(arena_t *const self);

#ifndef IGNORE_ARENA_IMPLEMENTATION

void *      arena__internal__reserve_memory_16byte_aligned(arena_t * const self, const u64 memory_size);
void *      arena__internal__reserve_tracked(arena_t *const self, const u64 memory_size, const char *file, int line, const char *func);

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
            arena__internal_giveback(self->meta.lifetime_owner, chunk, sizeof(free_chunks_t));
        } else {
            free(chunk);
        }
        return memory;
    }
    return memory;
}

void * arena__internal_reserve_memory_16byte_aligned(arena_t *const self, const u64 memory_size)
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

void * arena__internal__reserve_tracked(arena_t *const self, const u64 memory_size, const char *file, int line, const char *func)
{
    if (!self->meta.lifetime_owner)
        arena_logger_init(self, self->capacity, file, line);
    void *mem = arena__internal_reserve_memory_16byte_aligned(self, memory_size);
    if (mem) {
        arena_logger_log_alloc(self, memory_size, file, line, func);
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

void arena__internal__giveback_tracked(arena_t *const self, void *const ptr, const u64 size, const char *file, int line, const char *func)
{
    arena__internal_giveback(self, ptr, size);
    arena_logger_log_giveback(self, size, file, line, func);
}

arena_t * arena_init(arena_t *const arena, const u64 capacity)
{
    const u64 final_capacity = capacity + sizeof(arena_t);
    arena_t output = {
        .capacity = final_capacity,
        .size = 0,
        .memory = arena ? arena__internal_reserve_memory_16byte_aligned(arena, final_capacity) : calloc(final_capacity, sizeof(u8)),
        .freelist = {0},
        .meta = {
            .lifetime_owner = arena,
        },
    };
    atomic_flag_clear(&output.meta.lock);

    return arena_store(&output, &output, sizeof(arena_t));
}

void arena__internal_giveback(arena_t *const self, void *const ptr, const u64 size)
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

        //NOTE: using raw allocator directly instead of arena_store to avoid
        //tracking internal freelist metadata as user allocations in the arena dump
        free_chunks_t *new_chunk;
        if (self->meta.lifetime_owner) {
            new_chunk = (free_chunks_t *)arena__internal_reserve_memory_16byte_aligned(
                self->meta.lifetime_owner, sizeof(free_chunks_t)
            );
            if (new_chunk) {
                new_chunk->memory = ptr;
                new_chunk->size = size;
                new_chunk->next = NULL;
            }
        } else {
            new_chunk = calloc(1, sizeof(free_chunks_t));
            if (new_chunk) {
                new_chunk->memory = ptr;
                new_chunk->size = size;
                new_chunk->next = NULL;
            }
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
        self->size = sizeof(arena_t);
    atomic_flag_clear(&self->meta.lock);
    arena_logger_clear_log(self);
}

void arena_destroy(arena_t *const self)
{
    arena_logger_destroy(self);

    if (self->meta.lifetime_owner) {
        arena__internal_giveback(self->meta.lifetime_owner, self->memory, self->capacity);
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
    free(self->memory);
}


bool arena_is_init(const arena_t *const self)
{
    return self && (self->memory != NULL || self->capacity > 0);
}

static const char *al__fmt_bytes(u64 bytes, char *buf, u32 buf_sz)
{
    if (bytes >= MB) {
        snprintf(buf, buf_sz, "%.2f MB", (double)bytes / (double)MB);
    } else if (bytes >= KB) {
        snprintf(buf, buf_sz, "%.2f KB", (double)bytes / (double)KB);
    } else {
        snprintf(buf, buf_sz, "%lu B", (unsigned long)bytes);
    }
    return buf;
}

static u64 al__fl_bytes(arena_t *a)
{
    u64 sum = 0;
    for (free_chunks_t *fc = a->freelist.head; fc; fc = fc->next) sum += fc->size;
    return sum;
}

static void al__write_giveback_log(FILE *f, arena_t *a)
{
    arena_logger_t *l = arena_logger__internal_find(a);
    if (!l || !l->giveback_log_count) return;

    fprintf(f, "  Giveback log (%u entries):\n", l->giveback_log_count);
    for (arena_giveback_record_t *r = l->giveback_log_head; r; r = r->next) {
        char size_str[64];
        al__fmt_bytes(r->size, size_str, sizeof(size_str));
        fprintf(f, "    %s:%d (%s)  returned %s\n",
                r->file, r->line, r->func, size_str);
    }
    fprintf(f, "\n");
}

void arena_logger_dump_summary(const char *filepath)
{
    FILE *f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "[arena_logger] Failed to open '%s' for writing\n", filepath);
        return;
    }

    u64 total_capacity = 0, total_used = 0, total_unused = 0, total_fl_bytes = 0;
    u32 total_allocs = 0, total_givebacks = 0, total_fl_chunks = 0;
    u32 arena_count = 0;
    for (arena_logger_t *lg = arena_logger_registry; lg; lg = lg->next) arena_count++;

    fprintf(f, "================================================================================\n");
    fprintf(f, "                         ARENA MEMORY REPORT  (%u arenas)\n", arena_count);
    fprintf(f, "================================================================================\n\n");
    fprintf(f, "%-50s %12s %12s %12s %7s %7s %14s\n",
            "Arena (init location)", "Capacity", "Used", "Unused", "Allocs", "Gbacks", "Freelist(ch/B)");
    fprintf(f, "------------------------------------------------------------------------------------------------------------------------\n");

    for (arena_logger_t *lg = arena_logger_registry; lg; lg = lg->next) {
        arena_t *a = lg->arena;
        u64 used = a ? a->size : 0;
        u64 unused = lg->capacity > used ? lg->capacity - used : 0;
        u64 flb = a ? al__fl_bytes(a) : 0;
        u32 flc = a ? a->freelist.count : 0;

        char cap_str[32], used_str[32], unused_str[32], fl_str[64];
        al__fmt_bytes(lg->capacity, cap_str, sizeof(cap_str));
        al__fmt_bytes(used, used_str, sizeof(used_str));
        al__fmt_bytes(unused, unused_str, sizeof(unused_str));
        snprintf(fl_str, sizeof(fl_str), "%u / %lu B", flc, (unsigned long)flb);

        const char *sub = a && a->meta.lifetime_owner ? "(sub)" : "";
        char label[64];
        snprintf(label, sizeof(label), "%s:%d %s", lg->init_file ? lg->init_file : "?", lg->init_line, sub);

        fprintf(f, "%-50s %12s %12s %12s %7u %7u %14s\n",
                label, cap_str, used_str, unused_str,
                lg->alloc_log_count, lg->giveback_log_count, fl_str);

        total_capacity  += lg->capacity;
        total_used      += used;
        total_unused    += unused;
        total_allocs    += lg->alloc_log_count;
        total_givebacks += lg->giveback_log_count;
        total_fl_chunks += flc;
        total_fl_bytes  += flb;
    }

    char tcap_str[32], tused_str[32], tunused_str[32], tfl_str[64];
    al__fmt_bytes(total_capacity, tcap_str, sizeof(tcap_str));
    al__fmt_bytes(total_used, tused_str, sizeof(tused_str));
    al__fmt_bytes(total_unused, tunused_str, sizeof(tunused_str));
    snprintf(tfl_str, sizeof(tfl_str), "%u / %lu B", total_fl_chunks, (unsigned long)total_fl_bytes);

    fprintf(f, "------------------------------------------------------------------------------------------------------------------------\n");
    fprintf(f, "%-50s %12s %12s %12s %7u %7u %14s\n",
            "TOTAL", tcap_str, tused_str, tunused_str, total_allocs, total_givebacks, tfl_str);
    fprintf(f, "\n================================================================================\n\n");
    fprintf(f, "Usage breakdown by arena:\n");
    fprintf(f, "------------------------------------------------------------------------------------------------------------------------\n");

    for (arena_logger_t *lg = arena_logger_registry; lg; lg = lg->next) {
        arena_t *a = lg->arena;
        u64 used = a ? a->size : 0;
        const char *sub = a && a->meta.lifetime_owner ? "(sub)" : "";
        double pct_used = lg->capacity ? (double)used / (double)lg->capacity * 100.0 : 0.0;

        char used_str[32], cap_str[32];
        al__fmt_bytes(used, used_str, sizeof(used_str));
        al__fmt_bytes(lg->capacity, cap_str, sizeof(cap_str));

        fprintf(f, "  %s:%d %s\n", lg->init_file ? lg->init_file : "?", lg->init_line, sub);
        fprintf(f, "    capacity = %s  |  used = %s (%.1f%%)  |  allocs = %u  |  givebacks = %u\n",
                cap_str, used_str, pct_used, lg->alloc_log_count, lg->giveback_log_count);
        if (a && a->freelist.count) {
            fprintf(f, "    freelist: %u chunks, %lu bytes total\n", a->freelist.count, (unsigned long)al__fl_bytes(a));
        }
    }

    fprintf(f, "\n================================================================================\n");
    fprintf(f, "                        GIVEBACK DETAILS\n");
    fprintf(f, "================================================================================\n\n");

    for (arena_logger_t *lg = arena_logger_registry; lg; lg = lg->next) {
        if (!lg->giveback_log_count) continue;
        arena_t *a = lg->arena;
        const char *sub = a && a->meta.lifetime_owner ? "(sub)" : "";
        char cap_str[32];
        al__fmt_bytes(lg->capacity, cap_str, sizeof(cap_str));
        fprintf(f, "--- %s:%d %s (%s, %u givebacks) ---\n",
                lg->init_file ? lg->init_file : "?", lg->init_line, sub, cap_str, lg->giveback_log_count);
        al__write_giveback_log(f, a);
    }

    fclose(f);
}

void arena_logger_dump_json_simple(const char *filepath)
{
    FILE *f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "[arena_logger] Failed to open '%s' for writing\n", filepath);
        return;
    }

    fprintf(f, "[\n");
    bool first = true;
    for (arena_logger_t *lg = arena_logger_registry; lg; lg = lg->next) {
        arena_t *a = lg->arena;
        u64 flb = a ? al__fl_bytes(a) : 0;
        u64 used = a ? a->size : 0;
        u64 unused = lg->capacity > used ? lg->capacity - used : 0;

        if (!first) fprintf(f, ",\n");
        first = false;

        fprintf(f, "  {\n");
        fprintf(f, "    \"file\": \"%s\",\n", lg->init_file ? lg->init_file : "?");
        fprintf(f, "    \"line\": %d,\n", lg->init_line);
        fprintf(f, "    \"capacity\": %lu,\n", (unsigned long)lg->capacity);
        fprintf(f, "    \"used\": %lu,\n", (unsigned long)used);
        fprintf(f, "    \"unused\": %lu,\n", (unsigned long)unused);
        fprintf(f, "    \"alloc_count\": %u,\n", lg->alloc_log_count);
        fprintf(f, "    \"giveback_count\": %u,\n", lg->giveback_log_count);
        fprintf(f, "    \"freelist_chunks\": %u,\n", a ? a->freelist.count : 0);
        fprintf(f, "    \"freelist_bytes\": %lu,\n", (unsigned long)flb);
        fprintf(f, "    \"freelist_used_pct\": %.2f\n", lg->capacity ? ((double)flb / (double)lg->capacity) * 100.0 : 0.0);
        fprintf(f, "  }");
    }
    fprintf(f, "\n]\n");
    fclose(f);
}

#endif

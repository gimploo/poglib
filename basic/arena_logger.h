#pragma once

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "common.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__linux__)
    #include <dlfcn.h>
    #include <execinfo.h>
#elif defined(_WIN64)
    #include <windows.h>
    #include <imagehlp.h>
#endif

#define ARENA_BACKTRACE_DEPTH 8
#define ARENA_BACKTRACE_SKIP  2

/*=============================================================================
                         - ARENA ALLOCATION LOGGER -

  Lightweight profiling infrastructure for the arena allocator.
  Tracks every arena_reserve/arena_store call with file, line,
  function, byte count, and full backtrace.  Dumps aggregated
  Brendan Gregg collapsed stack format at shutdown.

  Usage:  arena.h pulls this in via dbg.h (arena.h → dbg.h →
  arena_logger.h).  No explicit include needed in user code.
================================================================================*/

typedef struct arena_t arena_t;

typedef struct arena_alloc_record arena_alloc_record_t;
struct arena_alloc_record {
    const char          *file;
    int                  line;
    const char          *func;
    uint64_t             size;
    char                *backtrace_frames[ARENA_BACKTRACE_DEPTH];
    u32                  backtrace_count;
    arena_alloc_record_t *next;
};

typedef struct arena_logger_t arena_logger_t;
struct arena_logger_t {
    arena_t            *arena;
    uint64_t            capacity;
    uint64_t            used;
    const char          *init_file;
    int                 init_line;
    arena_alloc_record_t *alloc_log_head;
    uint32_t             alloc_log_count;
    arena_logger_t      *next;
};

extern arena_logger_t *arena_logger_registry;

void    arena_logger_init(arena_t *const arena, uint64_t capacity, const char *file, int line);
void    arena_logger_destroy(arena_t *const arena);
void    arena_logger_log_alloc(arena_t *const arena, uint64_t size, const char *file, int line, const char *func);
void    arena_logger_clear_log(arena_t *const arena);
void    arena_logger_dump_collapsed(const char *filepath);

#ifndef IGNORE_ARENA_LOGGER_IMPLEMENTATION

arena_logger_t *arena_logger_registry = NULL;

static arena_logger_t *arena_logger__internal_find(arena_t *const arena)
{
    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        if (l->arena == arena) return l;
    }
    return NULL;
}

void arena_logger_init(arena_t *const arena, uint64_t capacity, const char *file, int line)
{
    if (arena_logger__internal_find(arena)) return;

    arena_logger_t *l = (arena_logger_t *)calloc(1, sizeof(*l));
    l->arena     = arena;
    l->capacity  = capacity;
    l->init_file = file;
    l->init_line = line;
    l->next      = arena_logger_registry;
    arena_logger_registry = l;
}

void arena_logger_destroy(arena_t *const arena)
{
    arena_logger_t *prev = NULL;
    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        if (l->arena != arena) { prev = l; continue; }

        if (prev)   prev->next = l->next;
        else        arena_logger_registry = l->next;

        arena_logger_clear_log(arena);
        free(l);
        return;
    }
}

void arena_logger_log_alloc(arena_t *const arena, uint64_t size, const char *file, int line, const char *func)
{
    arena_logger_t *l = arena_logger__internal_find(arena);
    if (!l) return;

    arena_alloc_record_t *rec = (arena_alloc_record_t *)calloc(1, sizeof(*rec));
    rec->file = file;
    rec->line = line;
    rec->func = func;
    rec->size = size;

#if defined(_WIN64)
    {
        void         *stack[ARENA_BACKTRACE_DEPTH + ARENA_BACKTRACE_SKIP + 1];
        unsigned short frames = CaptureStackBackTrace(0, ARENA_BACKTRACE_DEPTH + ARENA_BACKTRACE_SKIP + 1, stack, NULL);
        SYMBOL_INFO   *symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        symbol->MaxNameLen   = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);

        u32 count = 0;
        for (int i = ARENA_BACKTRACE_SKIP + 1; i < (int)frames && count < ARENA_BACKTRACE_DEPTH; i++) {
            if (SymFromAddr(process, (DWORD64)stack[i], 0, symbol)) {
                rec->backtrace_frames[count] = (char *)calloc(256, sizeof(char));
                snprintf(rec->backtrace_frames[count], 255, "%s", symbol->Name);
            } else {
                rec->backtrace_frames[count] = (char *)calloc(32, sizeof(char));
                snprintf(rec->backtrace_frames[count], 31, "0x%p", stack[i]);
            }
            count++;
        }
        rec->backtrace_count = count;
        free(symbol);
    }
#elif defined(__linux__)
    {
        void *array[ARENA_BACKTRACE_DEPTH + ARENA_BACKTRACE_SKIP + 1];
        int frame_count = backtrace(array, ARENA_BACKTRACE_DEPTH + ARENA_BACKTRACE_SKIP + 1);
        u32 count = 0;

        for (int i = ARENA_BACKTRACE_SKIP + 1; i < frame_count && count < ARENA_BACKTRACE_DEPTH; i++) {
            Dl_info info;
            if (dladdr(array[i], &info) && info.dli_sname) {
                rec->backtrace_frames[count] = (char *)calloc(256, sizeof(char));
                snprintf(rec->backtrace_frames[count], 255, "%s", info.dli_sname);
            } else {
                rec->backtrace_frames[count] = (char *)calloc(32, sizeof(char));
                snprintf(rec->backtrace_frames[count], 31, "%p", array[i]);
            }
            count++;
        }
        rec->backtrace_count = count;
    }
#else
    rec->backtrace_count = 0;
#endif

    rec->next = l->alloc_log_head;
    l->alloc_log_head = rec;
    l->alloc_log_count++;
    l->used += size;
}

void arena_logger_clear_log(arena_t *const arena)
{
    arena_logger_t *l = arena_logger__internal_find(arena);
    if (!l) return;

    arena_alloc_record_t *cur = l->alloc_log_head;
    while (cur) {
        arena_alloc_record_t *next = cur->next;
        for (u32 i = 0; i < cur->backtrace_count; i++) {
            free(cur->backtrace_frames[i]);
        }
        free(cur);
        cur = next;
    }
    l->alloc_log_head = NULL;
    l->alloc_log_count = 0;
    l->used = 0;
}

typedef struct {
    char     *stack_key;
    uint64_t  total_bytes;
    uint32_t  count;
} arena_logger__stack_entry;

void arena_logger_dump_collapsed(const char *filepath)
{
#ifdef ARENA_ENABLE_MEMORY_LOGGER
    FILE *f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "[arena_logger] Failed to open '%s' for writing\n", filepath);
        return;
    }

    uint64_t total_records = 0;
    uint32_t arena_count = 0;
    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        total_records += l->alloc_log_count;
        arena_count++;
    }

    uint64_t entries_cap = total_records > 0 ? total_records : 64;
    arena_logger__stack_entry *entries = (arena_logger__stack_entry *)calloc(entries_cap, sizeof(*entries));
    uint64_t entry_count = 0;

    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        char arena_key[512];
        snprintf(arena_key, sizeof(arena_key), "arena:%s:%d(cap=%lluB,used=%lluB)",
                 l->init_file ? l->init_file : "?", l->init_line,
                 (unsigned long long)l->capacity,
                 (unsigned long long)l->used);

        for (arena_alloc_record_t *r = l->alloc_log_head; r; r = r->next) {
            char key[4096];
            int pos = snprintf(key, sizeof(key), "%s:%d:%s",
                               r->file ? r->file : "?", r->line,
                               r->func ? r->func : "?");
            for (uint32_t b = 0; b < r->backtrace_count; b++) {
                pos += snprintf(key + pos, sizeof(key) - pos, ";%s",
                                r->backtrace_frames[b] ? r->backtrace_frames[b] : "?");
            }
            pos += snprintf(key + pos, sizeof(key) - pos, ";%s", arena_key);

            uint64_t idx = UINT64_MAX;
            for (uint64_t i = 0; i < entry_count; i++) {
                if (strcmp(entries[i].stack_key, key) == 0) {
                    idx = i;
                    break;
                }
            }

            if (idx != UINT64_MAX) {
                entries[idx].total_bytes += r->size;
                entries[idx].count++;
            } else {
                if (entry_count >= entries_cap) {
                    entries_cap *= 2;
                    entries = (arena_logger__stack_entry *)realloc(entries, entries_cap * sizeof(*entries));
                }
                size_t klen = strlen(key) + 1;
                entries[entry_count].stack_key = (char *)malloc(klen);
                memcpy(entries[entry_count].stack_key, key, klen);
                entries[entry_count].total_bytes = r->size;
                entries[entry_count].count = 1;
                entry_count++;
            }
        }
    }

    uint64_t total_capacity = 0;
    uint64_t total_used = 0;
    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        total_capacity += l->capacity;
        total_used += l->used;
    }

    uint64_t total_allocs = 0;
    for (uint64_t i = 0; i < entry_count; i++)
        total_allocs += entries[i].count;

    fprintf(stdout, "=============================================\n");
    fprintf(stdout, " Arena Allocation Map (collapsed stack format)\n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "Summary\n");
    fprintf(stdout, "  Arenas: %u\n", arena_count);
    fprintf(stdout, "  Total capacity: %llu B", (unsigned long long)total_capacity);
    if (total_capacity >= MB) fprintf(stdout, " (%llu MB)", (unsigned long long)(total_capacity / MB));
    fprintf(stdout, "\n");
    fprintf(stdout, "  Total used: %llu B", (unsigned long long)total_used);
    if (total_used >= MB) fprintf(stdout, " (%llu MB)", (unsigned long long)(total_used / MB));
    fprintf(stdout, "\n");
    fprintf(stdout, "  Unique stacks: %llu\n", (unsigned long long)entry_count);
    fprintf(stdout, "  Total allocations: %llu\n", (unsigned long long)total_allocs);
    fprintf(stdout, "=============================================\n");
#if 0
    fprintf(stdout, "# Per arena:\n");
    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        uint64_t free_bytes = l->capacity > l->used ? l->capacity - l->used : 0;
        fprintf(stdout, "#   %s:%d\n", l->init_file ? l->init_file : "?", l->init_line);
        fprintf(stdout, "#     capacity: %llu B", (unsigned long long)l->capacity);
        if (l->capacity >= MB) fprintf(stdout, " (%llu MB)", (unsigned long long)(l->capacity / MB));
        fprintf(stdout, "\n");
        fprintf(stdout, "#     used: %llu B", (unsigned long long)l->used);
        if (l->used >= MB) fprintf(stdout, " (%llu MB)", (unsigned long long)(l->used / MB));
        fprintf(stdout, "\n");
        fprintf(stdout, "#     free: %llu B", (unsigned long long)free_bytes);
        if (free_bytes >= MB) fprintf(stdout, " (%llu MB)", (unsigned long long)(free_bytes / MB));
        fprintf(stdout, "\n");
        fprintf(stdout, "#     allocations: %u\n", l->alloc_log_count);
    }
    fprintf(stdout, "#\n");
#endif

    for (uint64_t i = 0; i < entry_count; i++) {
        fprintf(f, "%s %llu %u\n", entries[i].stack_key,
                (unsigned long long)entries[i].total_bytes,
                entries[i].count);
    }

    for (uint64_t i = 0; i < entry_count; i++)
        free(entries[i].stack_key);
    free(entries);

    fclose(f);
#else
    fprintf(stderr, "[!] WARN: Memory logger not enabled - define `ARENA_ENABLE_MEMORY_LOGGER` in main.c before library import\n");
#endif
}

#endif // IGNORE_ARENA_LOGGER_IMPLEMENTATION

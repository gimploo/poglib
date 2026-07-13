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
  function, and byte count.  Dumps speedscope-compatible JSON
  flamegraphs at shutdown.

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
void    arena_logger_dump_json(const char *filepath);

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
}

static u32 arena_logger__internal_frame_lookup(const char *name, char fnames[][256], u32 *count)
{
    for (u32 i = 0; i < *count; i++) {
        if (strcmp(fnames[i], name) == 0) return i;
    }
    strncpy(fnames[*count], name, 255);
    fnames[*count][255] = '\0';
    return (*count)++;
}

void arena_logger_dump_json(const char *filepath)
{
    FILE *f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "[arena_logger] Failed to open '%s' for writing\n", filepath);
        return;
    }

    char fname[1024][256];
    u32  fc = 0;

    u64 total_capacity = 0;
    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        total_capacity += l->capacity;
    }

    const u64 TARGET_MAX = 1000000;
    double scale = total_capacity ? (double)TARGET_MAX / (double)total_capacity : 1.0;

    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s:%d (%lu MB)",
                 l->init_file ? l->init_file : "?", l->init_line,
                 (unsigned long)(l->capacity / MB));
        arena_logger__internal_frame_lookup(buf, fname, &fc);

        for (arena_alloc_record_t *r = l->alloc_log_head; r; r = r->next) {
            const char *sf = strrchr(r->file, '/');
            sf = sf ? sf + 1 : r->file;
            arena_logger__internal_frame_lookup(sf, fname, &fc);
            arena_logger__internal_frame_lookup(r->func, fname, &fc);
            snprintf(buf, sizeof(buf), "L%03d [%lu B]",
                     r->line, (unsigned long)r->size);
            arena_logger__internal_frame_lookup(buf, fname, &fc);
            for (u32 b = 0; b < r->backtrace_count; b++) {
                arena_logger__internal_frame_lookup(r->backtrace_frames[b], fname, &fc);
            }
        }
    }
    arena_logger__internal_frame_lookup("(unused)", fname, &fc);

    fprintf(f, "{\n");
    fprintf(f, "  \"$schema\": \"https://www.speedscope.app/file-format-schema.json\",\n");
    fprintf(f, "  \"shared\": {\n    \"frames\": [\n");
    for (u32 i = 0; i < fc; i++) {
        fprintf(f, "      {\"name\": \"%s\"}%s\n", fname[i], i + 1 < fc ? "," : "");
    }
    fprintf(f, "    ]\n  },\n");
    fprintf(f, "  \"profiles\": [{\n");
    fprintf(f, "    \"type\": \"evented\",\n");
    fprintf(f, "    \"unit\": \"none\",\n");
    fprintf(f, "    \"name\": \"Arena Memory Map\",\n");
    fprintf(f, "    \"startValue\": 0,\n");
    fprintf(f, "    \"endValue\": %lu,\n", (unsigned long)TARGET_MAX);
    fprintf(f, "    \"events\": [\n");

    bool first = true;
    u64  base  = 0;

    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s:%d (%lu MB)",
                 l->init_file ? l->init_file : "?", l->init_line,
                 (unsigned long)(l->capacity / MB));
        u32 arena_f = arena_logger__internal_frame_lookup(buf, fname, &fc);

        u64 norm_base = (u64)((double)base * scale);

        if (!first) fprintf(f, ",\n");
        fprintf(f, "      {\"type\": \"O\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_base, arena_f);
        first = false;

        u64 cur = base;
        for (arena_alloc_record_t *r = l->alloc_log_head; r; r = r->next) {
            const char *sf = strrchr(r->file, '/');
            sf = sf ? sf + 1 : r->file;
            u32 file_f   = arena_logger__internal_frame_lookup(sf, fname, &fc);
            u32 func_f   = arena_logger__internal_frame_lookup(r->func, fname, &fc);
            snprintf(buf, sizeof(buf), "L%03d [%lu B]", r->line, (unsigned long)r->size);
            u32 detail_f = arena_logger__internal_frame_lookup(buf, fname, &fc);

            u64 norm_cur = (u64)((double)cur * scale);
            cur += r->size;
            u64 norm_end = (u64)((double)cur * scale);

            for (i32 b = (i32)r->backtrace_count - 1; b >= 0; b--) {
                u32 bt_f = arena_logger__internal_frame_lookup(r->backtrace_frames[b], fname, &fc);
                fprintf(f, ",\n      {\"type\": \"O\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_cur, bt_f);
            }
            fprintf(f, ",\n      {\"type\": \"O\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_cur, file_f);
            fprintf(f, ",\n      {\"type\": \"O\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_cur, func_f);
            fprintf(f, ",\n      {\"type\": \"O\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_cur, detail_f);
            fprintf(f, ",\n      {\"type\": \"C\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_end, detail_f);
            fprintf(f, ",\n      {\"type\": \"C\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_end, func_f);
            fprintf(f, ",\n      {\"type\": \"C\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_end, file_f);
            for (u32 b = 0; b < r->backtrace_count; b++) {
                u32 bt_f = arena_logger__internal_frame_lookup(r->backtrace_frames[b], fname, &fc);
                fprintf(f, ",\n      {\"type\": \"C\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_end, bt_f);
            }
        }

        u64 end = base + l->capacity;
        u64 norm_end = (u64)((double)end * scale);

        if (cur < end) {
            u64 norm_cur = (u64)((double)cur * scale);
            u32 unused_f = arena_logger__internal_frame_lookup("(unused)", fname, &fc);
            fprintf(f, ",\n      {\"type\": \"O\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_cur, unused_f);
            fprintf(f, ",\n      {\"type\": \"C\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_end, unused_f);
        }

        fprintf(f, ",\n      {\"type\": \"C\", \"at\": %lu, \"frame\": %u}", (unsigned long)norm_end, arena_f);
        base = end;
    }

    fprintf(f, "\n    ]\n  }]\n}\n");
    fclose(f);
}

#endif // IGNORE_ARENA_LOGGER_IMPLEMENTATION

#pragma once
#include <stdint.h>
#include <stdlib.h>

/*=============================================================================
                         - ARENA ALLOCATION LOGGER -

  Lightweight profiling infrastructure for the arena allocator.
  Tracks every arena_reserve/arena_store call with file, line,
  function, and byte count.  Dumps speedscope-compatible JSON
  flamegraphs at shutdown.

  Usage:  include after arena.h (or let dbg.h pull it in via
  the arena.h → dbg.h → arena_logger.h include chain).
================================================================================*/

typedef struct arena_t arena_t;

struct arena_alloc_record {
    const char  *file;
    int          line;
    const char  *func;
    uint64_t     size;
    struct arena_alloc_record *next;
};
typedef struct arena_alloc_record arena_alloc_record_t;

typedef struct arena_logger_t {
    arena_t     *arena;
    const char  *init_file;
    int          init_line;
    struct {
        arena_alloc_record_t *head;
        uint32_t              count;
    } alloc_log;
    struct arena_logger_t *next;
} arena_logger_t;

extern arena_logger_t *arena_logger_registry;

void    arena_logger_init(arena_t *const arena, const char *file, int line);
void    arena_logger_destroy(arena_t *const arena);
void    arena_logger_log_alloc(arena_t *const arena, uint64_t size, const char *file, int line, const char *func);
void    arena_logger_clear_log(arena_t *const arena);

#ifndef IGNORE_ARENA_LOGGER_IMPLEMENTATION

arena_logger_t *arena_logger_registry = NULL;

static arena_logger_t *arena_logger__internal_find(arena_t *const arena)
{
    for (arena_logger_t *l = arena_logger_registry; l; l = l->next) {
        if (l->arena == arena) return l;
    }
    return NULL;
}

void arena_logger_init(arena_t *const arena, const char *file, int line)
{
    if (arena_logger__internal_find(arena)) return;

    arena_logger_t *l = (arena_logger_t *)calloc(1, sizeof(*l));
    l->arena     = arena;
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
    rec->next = l->alloc_log.head;
    l->alloc_log.head = rec;
    l->alloc_log.count++;
}

void arena_logger_clear_log(arena_t *const arena)
{
    arena_logger_t *l = arena_logger__internal_find(arena);
    if (!l) return;

    arena_alloc_record_t *cur = l->alloc_log.head;
    while (cur) {
        arena_alloc_record_t *next = cur->next;
        free(cur);
        cur = next;
    }
    l->alloc_log.head = NULL;
    l->alloc_log.count = 0;
}

#endif // IGNORE_ARENA_LOGGER_IMPLEMENTATION

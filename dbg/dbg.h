/*
 * This header prints debug info when allocating and freeing 
 * memory from the heap
 */
#ifndef _DBG_H_
#define _DBG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "dbg_list.h"

#define KB 1024

typedef struct dbg_t dbg_t;
struct dbg_t {

    FILE *fp;
    const char *fname;
    
    DList list;

};


// Global debug meta data struct
extern dbg_t debug; 


// Init function required to start the debugger
static inline bool dbg_init(char *fpath)
{
    assert(fpath);
    FILE *fp = fopen(fpath, "w");
    assert(fp);
    debug = (dbg_t) {
        .fp = fp,
        .fname = fpath,
        .list = DList_init()
    };
    fprintf(stdout, "DBG: Initalized\n");

    return true;
}

// Close function required to end the debugger
static inline void dbg_destroy()
{
    fprintf(stdout, "DBG: Concluded\n");
    DList_destory(&debug.list);


    if (debug.list.count == 0)
        fprintf(debug.fp, 
                "NO MEMORY LEAKS\n");
    else
        fprintf(debug.fp, 
                "MEMORY LEAK FOUND -> COUNT %0li\n", debug.list.count);

    fclose(debug.fp);
}


// Wrapper of common memory allocating function in stdlib 

#define malloc(n) debug_malloc((n), __FILE__, __LINE__, __func__)
#define realloc(p, n) debug_realloc((p), (n), __FILE__, __LINE__, __func__)
#define free(p) debug_free((p), (#p), __FILE__, __LINE__, __func__) 



/*
 *
 *
 *
 * Function Definition
 *
 *
 *
 *
 */


static void * debug_malloc(size_t size, char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp; // global variable
    DList *list = &debug.list;

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[0;32mMEMORY ALLOCTED (%0li bytes)\033[0m\n", 
            file_path, line_num, size);
    fprintf(fp, "\n");

#undef malloc

    void *malloc_mem = malloc(size);

#define malloc(n) debug_malloc((n), __FILE__, __LINE__, __func__)

    DNode *node = DNode_init(malloc_mem, DVT_ADDR);
    if (node == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }

    DList_append_node(list, node);
    
    return malloc_mem;
}


static void * debug_realloc(void *pointer, size_t size, char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp; // global variable
    DList *list = &debug.list;

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \tMEMORY \033[0;32m REALLOCTED \033[0m OF SIZE:%0li\n" ,
            file_path, line_num, size);
    fprintf(fp, "\n");

#undef realloc

    void *realloc_mem = realloc(pointer, size);

#define realloc(p, n) debug_realloc((p), (n), __FILE__, __LINE__, __func__)


    DNode *node = DNode_init(realloc_mem, DVT_ADDR);
    if (node == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }
    DList_append_node(&debug.list, node);
    
    return realloc_mem;
}


static void debug_free(void *pointer, char *pointer_name , char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp;
    DList *list = &debug.list;

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;34m'%s' DEALLOCATED\033[0m\n" 
            ,file_path, line_num, pointer_name);
    fprintf(fp, "\n");
    
    DList_delete_node_by_value(list, pointer);

#undef free

    free(pointer);

#define free(p) debug_free((p), (#p), __FILE__, __LINE__, __func__) 
}

#endif //_DEBUG_H_

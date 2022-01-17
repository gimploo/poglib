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

#include <string.h>

#define KB 1024

typedef struct dbg_t dbg_t;
struct dbg_t {

    FILE *fp;
    const char *fname;
    
    DList list;

};

// FIXME:
// Issue in DValMeta filename member, apparantely the filename isnt copied properly to it but in func_name that isnt a problem.


// Global debug meta data struct
extern dbg_t debug; 

bool dgb_init(dbg_t *debug, const char *);
void dbg_destroy();


// Wrapper of common memory allocating function in stdlib 

#define malloc(n) _debug_malloc((n), __FILE__, __LINE__, __func__)
#define realloc(p, n) _debug_realloc((p), (n), __FILE__, __LINE__, __func__)
#define free(p) _debug_free((p), (#p), __FILE__, __LINE__, __func__) 



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

// Init function required to start the debugger
bool dbg_init(dbg_t *debug, const char *fpath)
{
    assert(fpath);
    FILE *fp = fopen(fpath, "w");
    assert(fp);
    assert(debug);
    debug->fp = fp;
    debug->fname = fpath;
    debug->list = DList_init();

    fprintf(stdout, "DBG: Initalized\n");

    return true;
}


static void * _debug_malloc(size_t size, const char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp; // global variable
    DList *list = &debug.list;

    assert(fp); assert(list);

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[0;32mMEMORY ALLOCTED (%0li bytes)\033[0m\n", 
            file_path, line_num, size);
    fprintf(fp, "\n");

#undef malloc

    void *malloc_mem = malloc(size);

#define malloc(n) _debug_malloc((n), __FILE__, __LINE__, __func__)
    DValMeta meta = {0};

    meta.line_num = line_num;
    meta.size = size;
    memcpy(meta.func_name, func_name,strlen(func_name));
    memcpy(meta.filename, file_path, strlen(file_path));
    
    DNode *node = NULL;
    node = DNode_init(malloc_mem, DVT_ADDR, meta);

    if (node == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }

    DList_append_node(list, node);
    
    return malloc_mem;
}


static void * _debug_realloc(void *pointer, size_t size, const char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp; // global variable
    DList *list = &debug.list;

    assert(fp); assert(list);

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \tMEMORY \033[0;32m REALLOCTED \033[0m OF SIZE:%0li\n" ,
            file_path, line_num, size);
    fprintf(fp, "\n");

#undef realloc

    void *realloc_mem = realloc(pointer, size);

#define realloc(p, n) _debug_realloc((p), (n), __FILE__, __LINE__, __func__)


    DValMeta meta = {0};
    meta.line_num = line_num;
    meta.size = size;
    memcpy(meta.func_name, func_name, strlen(func_name));
    memcpy(meta.filename, file_path, strlen(file_path));

    DNode *node = NULL;
    node = DNode_init(realloc_mem, DVT_ADDR, meta);

    if (node == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }
    DList_append_node(list, node);
    
    return realloc_mem;
}


static void _debug_free(void *pointer, char *pointer_name , const char *file_path, size_t line_num, const char *func_name)
{
    FILE *fp = debug.fp;
    DList *list = &debug.list;

    assert(fp); assert(list);

    fprintf(fp, "%s: \tIn function '%s':\n", file_path, func_name);
    fprintf(fp, "%s:%li: \t\033[01;34m'%s' DEALLOCATED  \033[0m\n" 
            ,file_path, line_num, pointer_name);
    fprintf(fp, "\n");
    
    if (DList_delete_node_by_value(list, &pointer)) fprintf(stderr, "DList_delete_node_by_value: failed to find value\n");

#undef free

    free(pointer);

#define free(p) _debug_free((p), (#p), __FILE__, __LINE__, __func__) 
}

void _debug_mem_dump()
{
    DList *list = &debug.list;
    assert(list);

    DList_print(list);
    list = NULL;
    
}

// Close function required to end the debugger
void dbg_destroy()
{
    fprintf(stdout, "DBG: Concluded\n");


    DList_destory(&debug.list);


    if (debug.list.count == 0) {

        fprintf(debug.fp, 
                "NO MEMORY LEAKS\n");
        fprintf(stdout, 
                "NO MEMORY LEAKS\n");
    } else {

        fprintf(debug.fp, 
                "MEMORY LEAK FOUND -> COUNT %0li\n", debug.list.count);
        fprintf(stdout, 
                "MEMORY LEAK FOUND -> COUNT %0li\n", debug.list.count);
        _debug_mem_dump();
    }


    fclose(debug.fp);
}

#endif //_DEBUG_H_
